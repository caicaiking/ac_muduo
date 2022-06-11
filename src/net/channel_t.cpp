//
// Created by acai on 6/9/22.
//

#include "channel_t.h"
#include <poll.h>
#include <sstream>
#include "base/logging.h"
#include "event_loop_t.h"

namespace ac_muduo::net {

    const int channel_t::k_none_event = 0;
    const int channel_t::k_read_event = POLLIN | POLLPRI;
    const int channel_t::k_write_event = POLLOUT;

    channel_t::channel_t(event_loop_t *loop, int fd) :
            loop_(loop),
            fd_(fd),
            events_(0),
            revents_(0),
            index_(-1),
            log_hup_(false),
            tied_(false),
            event_handling_(false),
            add_to_loop_(false) {

    }

    channel_t::~channel_t() {
        assert(!event_handling_);
        assert(!add_to_loop_);

        if (this->loop_->is_in_loop_thread()) {
            assert(!this->loop_->has_channel(this));
        }

    }

    void channel_t::tie(const std::shared_ptr<void> &obj) {
        this->tie_ = obj;
        this->tied_ = true;

    }

    void channel_t::handle_event(timestamp receive_time) {

        if (this->tied_) {
            auto guard = this->tie_.lock();
            if (guard) {
                this->hand_event_with_guard(receive_time);
            }
        } else {
            this->hand_event_with_guard(receive_time);
        }

    }

    std::string channel_t::revent_to_string() {
        return this->events_to_string(this->fd_, this->revents_);
    }

    std::string channel_t::event_to_string() {
        return this->events_to_string(this->fd_, this->events_);
    }

    void channel_t::remove() {
        assert(this->is_none_events());
        add_to_loop_ = false;
        this->loop_->remove_channel(this);
    }

    string channel_t::events_to_string(int fd, int ev) {
        std::ostringstream oss;
        oss << fd << ": ";
        if (ev & POLLIN)
            oss << "IN ";
        if (ev & POLLPRI)
            oss << "PRI ";
        if (ev & POLLOUT)
            oss << "OUT ";
        if (ev & POLLHUP)
            oss << "HUP ";
        if (ev & POLLRDHUP)
            oss << "RDHUP ";
        if (ev & POLLERR)
            oss << "ERR ";
        if (ev & POLLNVAL)
            oss << "NVAL ";

        return oss.str();
    }

    void channel_t::update() {
        this->add_to_loop_ = true;
        this->loop_->update_channel(this);
    }

    void channel_t::hand_event_with_guard(timestamp receive_time) {
        this->event_handling_ = true;

        LOG_TRACE << this->revent_to_string();

        if ((this->revents_ & POLLHUP) && !(this->revents_ & POLLIN)) {
            if (this->log_hup_) {
                LOG_WARN << "fd = " << this->fd_ << " channel::handle_event() POLLHUP";
            }
            if (this->close_callback_) this->close_callback_();
        }

        if (this->revents_ & POLLNVAL) {
            LOG_WARN << "fd = " << this->fd_ << " channel::handle_event() POLLNVAL";
        }

        if (this->revents_ & (POLLERR | POLLNVAL)) {
            if (this->error_callback_) this->error_callback_();
        }

        if (this->revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
            if (this->read_callback_) this->read_callback_(receive_time);
        }

        if (this->revents_ & POLLOUT) {
            if (this->write_callback_) write_callback_();
        }

        this->event_handling_ = false;
    }
}