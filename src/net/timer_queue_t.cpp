//
// Created by acai on 6/11/22.
//
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "timer_queue_t.h"
#include "base/logging.h"
#include "event_loop_t.h"
#include "timer_t.h"
#include "timer_id_t.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <ctime>

namespace ac_muduo::net {
    namespace detail {
        int create_timer_fd() {
            int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
            if (timer_fd < 0) {
                LOG_SYSFATAL << "failed in create_timer_fd";
            }
            return timer_fd;
        }

        struct timespec how_much_time_from_now(timestamp when) {
            auto microseconds = when.micro_seconds_since_epoch() - timestamp::now().micro_seconds_since_epoch();
            if (microseconds < 100) {
                microseconds = 100;
            }

            struct timespec ts;
            ts.tv_sec = microseconds / timestamp::k_micro_seconds_per_second;
            ts.tv_nsec = static_cast<long>((microseconds % timestamp::k_micro_seconds_per_second) * 1000);
            return ts;
        }

        void read_timer_fd(int timer_fd, timestamp now) {
            uint64_t howmany;
            ssize_t n = ::read(timer_fd, &howmany, sizeof(howmany));
            LOG_TRACE << "timer_queue::handle_read() " << howmany << " at " << now.to_string();

            if (n != sizeof howmany) {
                LOG_ERROR << "timer_queue::handle_read() reads " << n << " bytes instead of 8";
            }
        }

        void reset_timer_fd(int timer_fd, timestamp expiration) {
            struct itimerspec new_value;
            struct itimerspec old_value;

            mem_zero(&new_value, sizeof new_value);
            mem_zero(&old_value, sizeof old_value);

            new_value.it_value = how_much_time_from_now(expiration);
            int ret = ::timerfd_settime(timer_fd, 0, &new_value, &old_value);
            if (ret) {
                LOG_SYSERR << "timerfd_settime()";
            }
        }
    }

    timer_queue_t::timer_queue_t(event_loop_t *loop) :
            loop_(loop),
            timer_fd_(detail::create_timer_fd()),
            timer_fd_channel_(loop, timer_fd_),
            timers_(),
            calling_expired_timers_(false) {
        this->timer_fd_channel_.set_read_callback([this](timestamp t) {
            this->handle_read();
        });
        this->timer_fd_channel_.enable_reading();
    }

    timer_queue_t::~timer_queue_t() {
        this->timer_fd_channel_.disable_all();
        this->timer_fd_channel_.remove();
        ::close(this->timer_fd_);

        for (auto &timer: this->timers_) {
            delete timer.second;
        }
    }

    timer_id_t timer_queue_t::add_timer(timer_queue_t::timer_callback_t cb, timestamp when, double interval) {
        auto timer = new timer_t(std::move(cb), when, interval);

        this->loop_->run_in_loop([this, timer] {
            this->add_timer_in_loop(timer);
        });

        return timer_id_t(timer, timer->sequence());
    }

    void timer_queue_t::cancel(timer_id_t timer_id) {
        this->loop_->run_in_loop([this, timer_id] {
            this->cancel_in_loop(timer_id);
        });
    }

    void timer_queue_t::add_timer_in_loop(timer_t *timer) {
        this->loop_->assert_in_loop_thread();
        bool earliest_changed = this->insert(timer);

        if (earliest_changed) {
            ac_muduo::net::detail::reset_timer_fd(this->timer_fd_, timer->expiration());
        }
    }

    void timer_queue_t::cancel_in_loop(timer_id_t timer_id) {
        this->loop_->assert_in_loop_thread();
        assert(this->timers_.size() == this->active_timers_.size());

        active_timer_t timer(timer_id.timer_, timer_id.sequence_);
        auto iter = this->active_timers_.find(timer);

        if (iter != this->active_timers_.end()) {
            size_t n = this->timers_.erase({iter->first->expiration(), iter->first});
            assert(n == 1);
            (void) n;
            delete iter->first;
            this->active_timers_.erase(iter);
        } else if (this->calling_expired_timers_) {
            this->canceling_timers_.insert(timer);
        }
        assert(this->timers_.size() == this->active_timers_.size());
    }

    void timer_queue_t::handle_read() {

        this->loop_->assert_in_loop_thread();
        timestamp now(timestamp::now());
        ac_muduo::net::detail::read_timer_fd(this->timer_fd_, now);

        auto expired = this->get_expired(now);

        this->calling_expired_timers_ = true;
        this->canceling_timers_.clear();

        for (auto &it: expired) {
            it.second->run();
        }

        this->calling_expired_timers_ = false;
        this->reset(expired, now);
    }

    std::vector<timer_queue_t::entry_t> timer_queue_t::get_expired(timestamp now) {

        assert(this->timers_.size() == this->active_timers_.size());
        std::vector<entry_t> expired;

        entry_t sentry(now, reinterpret_cast<timer_t *>(UINTPTR_MAX));
        auto end = this->timers_.lower_bound(sentry);

        assert(end == this->timers_.end() || now < end->first);
        std::copy(timers_.begin(), end, std::back_inserter(expired));
        this->timers_.erase(this->timers_.begin(), end);

        for (auto &it: expired) {
            active_timer_t timer(it.second, it.second->sequence());
            auto n = this->active_timers_.erase(timer);
            assert(n == 1);
            (void) n;
        }

        assert(this->timers_.size() == this->active_timers_.size());
        return expired;
    }

    void timer_queue_t::reset(const std::vector<entry_t> &expired, timestamp now) {
        timestamp next_expire;
        for (auto &it: expired) {
            active_timer_t timer(it.second, it.second->sequence());
            if (it.second->repeat() &&
                (this->canceling_timers_.find(timer) == this->canceling_timers_.end())) {
                it.second->restart(now);
                this->insert(it.second);
            } else {
                delete it.second;
            }
        }

        if (!this->timers_.empty()) {
            next_expire = this->timers_.begin()->second->expiration();
        }
        if (next_expire.valid()) {
            detail::reset_timer_fd(this->timer_fd_, next_expire);
        }
    }

    bool timer_queue_t::insert(timer_t *timer) {
        this->loop_->assert_in_loop_thread();
        assert(this->timers_.size() == this->active_timers_.size());

        bool earliest_changed = false;
        auto when = timer->expiration();

        auto iter = this->timers_.begin();
        if (iter == this->timers_.end() || when < iter->first) {
            earliest_changed = true;
        }

        {
            auto result = this->timers_.insert(entry_t(when, timer));
            assert(result.second);
            (void) result;
        }
        {
            auto result = this->active_timers_.insert(active_timer_t(timer, timer->sequence()));
            assert(result.second);
            (void) result;
        }

        assert(this->timers_.size() == this->active_timers_.size());
        return earliest_changed;
    }
}