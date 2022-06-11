//
// Created by acai on 6/9/22.
//

#include "epoll_poller_t.h"
#include "base/logging.h"
#include "net/channel_t.h"

#include <cassert>
#include <cerrno>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

namespace ac_muduo::net {

    static_assert(EPOLLIN == POLLIN, "epoll uses the same flag values as poll");
    static_assert(EPOLLPRI == POLLPRI, "epoll uses the same flag values as poll");
    static_assert(EPOLLOUT == POLLOUT, "epoll uses the same flag values as poll");
    static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses the same flag values as poll");
    static_assert(EPOLLERR == POLLERR, "epoll uses the same flag values as poll");
    static_assert(EPOLLHUP == POLLHUP, "epoll uses the same flag values as poll");

    namespace {
        const int k_new = -1;
        const int k_added = 1;
        const int k_deleted = 2;
    }


    epoll_poller_t::epoll_poller_t(event_loop_t *loop) : poller_t(loop),
                                                         epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
                                                         events_(k_init_event_list_size) {
        if (this->epoll_fd_ < 0) {
            LOG_SYSFATAL << "epoll_poller_t::epoll_poller_t";
        }
    }

    epoll_poller_t::~epoll_poller_t() {
        ::close(this->epoll_fd_);
    }

    timestamp epoll_poller_t::poll(int timeout_ms, poller_t::channel_list_t *active_channels) {
        LOG_TRACE << "fd total count " << this->channels_.size();
        int num_events = ::epoll_wait(epoll_fd_, this->events_.data(), this->events_.size(), timeout_ms);

        int saved_errno = errno;
        timestamp now(timestamp::now());

        if (num_events > 0) {
            LOG_TRACE << num_events << " events happened";
            this->fill_active_channels(num_events, active_channels);
            if (num_events == this->events_.size()) {
                events_.resize(this->events_.size() * 2);
            }
        } else if (num_events == 0) {
            LOG_TRACE << "nothing happened";
        } else {
            if (saved_errno != EINTR) {
                errno = saved_errno;
                LOG_SYSERR << "epoll_poller::poll()";
            }
        }
        return now;
    }

    void epoll_poller_t::update_channel(channel_t *channel) {
        poller_t::assert_in_loop_thread();
        const int index = channel->index();
        LOG_TRACE << "fd = " << channel->fd() << " events= " << channel->events() << " index = " << index;

        if (index == k_new || index == k_deleted) {
            int fd = channel->fd();
            if (index == k_new) {
                assert(this->channels_.find(fd) == this->channels_.end());
                this->channels_[fd] = channel;
            } else {
                assert(this->channels_.find(fd) != this->channels_.end());
                assert(this->channels_[fd] = channel);
            }
            channel->set_index(k_added);
            this->update(EPOLL_CTL_ADD, channel);
        } else {
            int fd = channel->fd();
            (void) fd;
            assert(this->channels_.find(fd) != this->channels_.end());
            assert(this->channels_[fd] == channel);
            assert(index == k_added);

            if (channel->is_none_events()) {
                update(EPOLL_CTL_DEL, channel);
                channel->set_index(k_deleted);
            } else {
                update(EPOLL_CTL_MOD, channel);
            }
        }
    }

    void epoll_poller_t::remove_channel(channel_t *channel) {
        poller_t::assert_in_loop_thread();
        int fd = channel->fd();
        LOG_TRACE << "fd = " << fd;
        assert(this->channels_.find(fd) != this->channels_.end());
        assert(this->channels_[fd] == channel);
        assert(channel->is_none_events());
        int index = channel->index();
        assert(index == k_added || index == k_deleted);
        size_t n = this->channels_.erase(fd);
        (void) n;
        assert(n == 1);

        if (index == k_added) {
            update(EPOLL_CTL_DEL, channel);
        }
        channel->set_index(k_new);
    }

    const char *epoll_poller_t::operation_to_string(int op) {
        switch (op) {
            case EPOLL_CTL_ADD:
                return "ADD";
            case EPOLL_CTL_DEL:
                return "DEL";
            case EPOLL_CTL_MOD:
                return "MOD";

            default:
                assert(false && "ERROR op");
                return "unknown operation";
        }
    }

    void epoll_poller_t::fill_active_channels(int num_events,
                                              poller_t::channel_list_t *active_channels) const {
        assert(implicit_cat<size_t>(num_events) <= events_.size());

        for (int i = 0; i < num_events; ++i) {
            auto *channel = static_cast<channel_t *>(events_[i].data.ptr);
            channel->set_revents(events_[i].events);
            active_channels->push_back(channel);
        }
    }

    void epoll_poller_t::update(int operation, channel_t *channel) {
        struct epoll_event event;
        mem_zero(&event, sizeof(event));
        event.events = channel->events();
        event.data.ptr = channel;

        int fd = channel->fd();
        LOG_TRACE << "epoll_ctl op = " << operation_to_string(operation)
                  << " fd = " << fd << " event = {" << channel->event_to_string() << " }";

        if (::epoll_ctl(this->epoll_fd_, operation, fd, &event) < 0) {
            if (operation == EPOLL_CTL_DEL) {
                LOG_SYSERR << "epoll_ctl op = " << operation_to_string(operation) << " fd = " << fd;
            } else {
                LOG_SYSFATAL << "epoll_ctl op = " << operation_to_string(operation) << " fd = " << fd;
            }
        }
    }
}