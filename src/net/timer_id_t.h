//
// Created by acai on 6/11/22.
//

#ifndef AC_MUDUO_TIMER_ID_T_H
#define AC_MUDUO_TIMER_ID_T_H

#include "timer_t.h"
#include "base/copyable.h"

namespace ac_muduo::net {
    class timer_id_t : public copyable {
    public:
        timer_id_t() : timer_(nullptr), sequence_(0) {}

        timer_id_t(timer_t *timer, int64_t seq)
                : timer_(timer), sequence_(seq) {
        }

        friend class timer_queue_t;

    private:
        timer_t *timer_;
        int64_t sequence_;
    };
}

#endif //AC_MUDUO_TIMER_ID_T_H
