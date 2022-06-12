//
// Created by acai on 6/9/22.
//

#include "event_loop_t.h"
#include "channel_t.h"
#include "base/logging.h"
#include "poller_t.h"
#include "channel_t.h"
#include <algorithm>
#include <sys/eventfd.h>
#include <unistd.h>
#include <csignal>
#include "timer_queue_t.h"


namespace ac_muduo::net {

    namespace {
        __thread event_loop_t *t_loop_in_this_thread = nullptr;
        const int k_poll_time_ms = 1000;

        int create_eventfd() {
            int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            if (evtfd < 0) {
                LOG_SYSERR << "failed in eventfd";
                abort();
            }
            return evtfd;
        }

        class ignore_sigpipe_t {
        public:
            ignore_sigpipe_t() {
                ::signal(SIGPIPE, SIG_IGN);
            }
        };

        ignore_sigpipe_t init_obj;
    }


    event_loop_t::event_loop_t() :
            looping_(false),
            quit_(false),
            event_handling_(false),
            calling_pending_functors_(false),
            iteration_(0),
            thread_id_(current_thread::tid()),
            poller_(poller_t::new_default_poller(this)),
            timer_queue_(new timer_queue_t(this)),
            wakeup_fd_(create_eventfd()),
            wakeup_channel_(new channel_t(this, wakeup_fd_)),
            current_active_channel_(nullptr) {
        LOG_DEBUG << "event_loop created " << this << " in thread " << this->thread_id_;
        if (t_loop_in_this_thread) {
            LOG_FATAL << "Another event_loop " << t_loop_in_this_thread << " exist in this thread " << this->thread_id_;
        } else {
            t_loop_in_this_thread = this;
        }

        this->wakeup_channel_->set_read_callback([this](timestamp n) { this->handle_read(); });
        this->wakeup_channel_->enable_reading();
    }

    event_loop_t::~event_loop_t() {
        LOG_DEBUG << "event_loop " << this << " of thread " << thread_id_ << " destructs in thread "
                  << current_thread::tid();
        this->wakeup_channel_->disable_all();
        this->wakeup_channel_->remove();
        ::close(this->wakeup_fd_);
        t_loop_in_this_thread = nullptr;
    }

    void event_loop_t::loop() {
        assert(!this->looping_);
        this->assert_in_loop_thread();

        this->looping_ = true;
        this->quit_ = false;

        LOG_TRACE << "event_loop " << this << " start looping";

        while (!quit_) {
            this->active_channels_.clear();
            this->poll_return_time_ = poller_->poll(k_poll_time_ms, &this->active_channels_);
            ++iteration_;

            if (logger::log_level() <= logger::log_level_t::TRACE) {
                this->print_active_channels();
            }

            this->event_handling_ = true;
            for (auto channel: this->active_channels_) {
                this->current_active_channel_ = channel;
                this->current_active_channel_->handle_event(this->poll_return_time_);
            }
            this->current_active_channel_ = nullptr;
            this->event_handling_ = false;
            do_pending_functors();
        }
        LOG_TRACE << "event_loop " << this << " stop looping";
        this->looping_ = false;
    }

    void event_loop_t::quit() {

        this->quit_ = true;
        if (!this->is_in_loop_thread()) {
            this->wakeup();
        }
    }

    void event_loop_t::run_in_loop(event_loop_t::functor_t cb) {
        if (this->is_in_loop_thread()) {
            cb();
        } else {
            this->queue_in_loop(std::move(cb));
        }
    }

    void event_loop_t::queue_in_loop(event_loop_t::functor_t cb) {
        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->pending_functors_.push_back(std::move(cb));
        }

        if (!this->is_in_loop_thread() || calling_pending_functors_) {
            this->wakeup();
        }

    }

    size_t event_loop_t::queue_size() const {
        std::lock_guard<std::mutex> lock(this->mutex_);
        return this->pending_functors_.size();
    }

    void event_loop_t::wakeup() {
        uint64_t one = 1;
        ssize_t n = ::write(this->wakeup_fd_, &one, sizeof one);
        if (n != sizeof one) {
            LOG_ERROR << "event_loop::wakeup() writes " << n << " bytes instead of 8";
        }
    }

    void event_loop_t::update_channel(channel_t *channel) {
        assert(channel->owner_loop() == this);
        this->assert_in_loop_thread();
        this->poller_->update_channel(channel);
    }

    void event_loop_t::remove_channel(channel_t *channel) {
        assert(channel->owner_loop() == this);
        this->assert_in_loop_thread();
        if (this->event_handling_) {
            assert(
                    this->current_active_channel_ == channel ||
                    std::find(this->active_channels_.begin(), this->active_channels_.end(), channel) ==
                    active_channels_.end()
            );
        }
        this->poller_->remove_channel(channel);
    }

    bool event_loop_t::has_channel(channel_t *channel) {
        assert(channel->owner_loop() == this);
        this->assert_in_loop_thread();
        return this->poller_->has_channel(channel);
    }

    event_loop_t *event_loop_t::get_event_loop_of_current_thread() {
        return t_loop_in_this_thread;
    }

    void event_loop_t::abort_not_in_loop_thread() {
        LOG_FATAL << "event_loop::abort_not_in_loop_thread - event_loop " << this
                  << " was created in thread_id_ = " << this->thread_id_
                  << ", current thread id = " << current_thread::tid();
    }

    void event_loop_t::handle_read() {
        uint64_t one = 1;
        ssize_t n = ::read(this->wakeup_fd_, &one, sizeof one);
        if (n != sizeof one) {
            LOG_ERROR << "event_loop::handle_read() reads " << n << " bytes instead of 8";
        }
    }

    void event_loop_t::do_pending_functors() {
        std::vector<functor_t> functors;
        this->calling_pending_functors_ = true;

        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            functors.swap(this->pending_functors_);
        }

        for (const auto &functor: functors) {
            functor();
        }

        this->calling_pending_functors_ = false;
    }

    void event_loop_t::print_active_channels() const {
        for (const auto &channel: this->active_channels_) {
            LOG_TRACE << "{" << channel->revent_to_string() << "} ";
        }
    }

    timer_id_t event_loop_t::run_at(timestamp time, std::function<void()> cb) {
        return this->timer_queue_->add_timer(std::move(cb), time, .0);
    }

    timer_id_t event_loop_t::run_after(double delay, std::function<void()> cb) {
        timestamp time(add_time(timestamp::now(), delay));
        return this->run_at(time, std::move(cb));
    }

    timer_id_t event_loop_t::run_every(double interval, std::function<void()> cb) {
        timestamp time(add_time(timestamp::now(), interval));
        return this->timer_queue_->add_timer(std::move(cb), time, interval);
    }

    void event_loop_t::cancel(timer_id_t timer_id) {
        this->timer_queue_->cancel(timer_id);
    }
}