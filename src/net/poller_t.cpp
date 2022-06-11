//
// Created by acai on 6/9/22.
//

#include "poller_t.h"
#include "channel_t.h"

namespace ac_muduo {
    namespace net {

        poller_t::poller_t(event_loop_t *loop) : ower_loop_(loop) {

        }

        poller_t::~poller_t() = default;

        bool poller_t::has_channel(channel_t *channel) {
            this->assert_in_loop_thread();

            auto iter = this->channels_.find(channel->fd());
            return iter != this->channels_.end() && iter->second == channel;
        }
    }
}