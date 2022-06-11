//
// Created by acai on 6/9/22.
//

#ifndef AC_MUDUO_POLL_POLLER_T_H
#define AC_MUDUO_POLL_POLLER_T_H

#include "net/poller_t.h"

struct pollfd;

namespace ac_muduo::net {
    class poll_poller_t : public poller_t {
    public:
        poll_poller_t(event_loop_t *loop);

        ~poll_poller_t();

        timestamp poll(int timeout_ms, channel_list_t *active_channels) override;

        void update_channel(channel_t *channel) override;

        void remove_channel(channel_t *channel) override;

    private:
        void fill_active_channels(int num_events, channel_list_t *active_channels) const;

        typedef std::vector<struct pollfd> pollfd_list_t;
        pollfd_list_t poll_fds_;

    };

}

#endif //AC_MUDUO_POLL_POLLER_T_H
