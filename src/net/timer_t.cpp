//
// Created by acai on 6/11/22.
//

#include "timer_t.h"

std::atomic_int64_t ac_muduo::net::timer_t::s_num_created_ = {};

void ac_muduo::net::timer_t::restart(ac_muduo::timestamp now) {
    if (this->repeat_) {
        expiration_ = add_time(now, interval_);
    } else {
        this->expiration_ = timestamp::invalid();
    }
}
