//
// Created by acai on 6/9/22.
//

#include "poll_poller_t.h"
#include "base/logging.h"
#include "base/types.h"
#include "net/channel_t.h"


#include <cassert>
#include <sys/poll.h>

namespace ac_muduo::net {

    poll_poller_t::poll_poller_t(event_loop_t *loop)
            : poller_t(loop) {

    }

    poll_poller_t::~poll_poller_t() = default;


    timestamp poll_poller_t::poll(int timeout_ms, poller_t::channel_list_t *active_channels) {
        int num_events = ::poll(this->poll_fds_.data(), this->poll_fds_.size(), timeout_ms);
        int saved_errno = errno;
        timestamp now(timestamp::now());

        if (num_events > 0) {
            LOG_TRACE << num_events << " events happened";
            this->fill_active_channels(num_events, active_channels);
        } else if (num_events == 0) {
            LOG_TRACE << " nothing happened";
        } else {
            if (saved_errno != EINTR) {
                errno = saved_errno;
                LOG_SYSERR << "poll_poller_t::poll()";
            }
        }
        return now;
    }

    void poll_poller_t::update_channel(channel_t *channel) {

        poller_t::assert_in_loop_thread();
        LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
        if (channel->index() < 0) {
            //add new one
            assert(this->channels_.find(channel->fd()) == this->channels_.end());
            struct pollfd pfd;
            pfd.fd = channel->fd();
            pfd.events = static_cast<short>(channel->events());
            pfd.revents = 0;
            this->poll_fds_.push_back(pfd);
            int index = static_cast<int>(this->poll_fds_.size()) - 1;
            channel->set_index(index);
            this->channels_[pfd.fd] = channel;
        } else {
            assert(this->channels_.find(channel->fd()) != this->channels_.end());
            assert(this->channels_[channel->fd()] == channel);

            int index = channel->index();
            assert(0 <= index && index < static_cast<int>(this->poll_fds_.size()));

            auto &pfd = this->poll_fds_[index];
            assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);

            pfd.fd = channel->fd();
            pfd.events = static_cast<short >(channel->events());
            pfd.revents = 0;

            if (channel->is_none_events()) {
                pfd.fd = -channel->fd() - 1;
            }
        }
    }

    void poll_poller_t::remove_channel(channel_t *channel) {
        poller_t::assert_in_loop_thread();
        LOG_TRACE << "fd = " << channel->fd();
        assert(this->channels_.find(channel->fd()) != this->channels_.end());
        assert(this->channels_[channel->fd()] == channel);
        assert(channel->is_none_events());

        int index = channel->index();
        assert(0 <= index && index < static_cast<int>(this->poll_fds_.size()));
        auto &pfd = this->poll_fds_[index];
        (void) pfd;
        assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());

        auto n = this->channels_.erase(channel->fd());
        assert(n == 1);
        (void) n;


        if (static_cast<int>(index) == this->poll_fds_.size() - 1) {
            this->poll_fds_.pop_back();
        } else {
            int channel_at_end = this->poll_fds_.back().fd;
            std::iter_swap(this->poll_fds_.begin() + index, this->poll_fds_.end() - 1);
            if (channel_at_end < 0) {
                channel_at_end = -channel_at_end - 1;
            }

            this->channels_[channel_at_end]->set_index(index);
            this->poll_fds_.pop_back();
        }
    }

    void poll_poller_t::fill_active_channels(int num_events, poller_t::channel_list_t *active_channels) const {

        for (auto pdf = this->poll_fds_.begin(); pdf != this->poll_fds_.end() && num_events > 0; ++pdf) {
            if (pdf->revents > 0) {
                --num_events;
                auto ch = this->channels_.find(pdf->fd);
                assert(ch != this->channels_.end());

                auto channel = ch->second;
                assert(channel->fd() == pdf->fd);
                channel->set_revents(pdf->revents);
                active_channels->push_back(channel);
            }
        }
    }
}