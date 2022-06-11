//
// Created by acai on 6/9/22.
//

#ifndef AC_MUDUO_EPOLL_POLLER_T_H
#define AC_MUDUO_EPOLL_POLLER_T_H

#include "net/poller_t.h"

struct epoll_event;

namespace ac_muduo::net {
    class epoll_poller_t : public poller_t {
    public:
        epoll_poller_t(event_loop_t *loop);

        ~epoll_poller_t() override;

        timestamp poll(int timeout_ms, channel_list_t *active_channels) override;

        void update_channel(channel_t *channel) override;

        void remove_channel(channel_t *channel) override;

    private:
        static const int k_init_event_list_size = 16;

        static const char *operation_to_string(int op);

        void fill_active_channels(int num_events, channel_list_t *active_channels) const;

        void update(int operation, channel_t *channel);

        typedef std::vector<struct epoll_event> event_list_t;

        int epoll_fd_;
        event_list_t events_;


    };
}


#endif //AC_MUDUO_EPOLL_POLLER_T_H
