//
// Created by acai on 6/9/22.
//

#include "net/poller_t.h"
#include "epoll_poller_t.h"
#include "poll_poller_t.h"

#include <stdlib.h>

namespace ac_muduo::net {
    poller_t *poller_t::new_default_poller(event_loop_t *loop) {
        if (::getenv("AC_MUDUO_USE_POLL")) {
            return new poll_poller_t(loop);
        } else {
            return new epoll_poller_t(loop);
        }
    }
}
