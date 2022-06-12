//
// Created by acai on 6/11/22.
//

#ifndef AC_MUDUO_TIMER_T_H
#define AC_MUDUO_TIMER_T_H

#include <atomic>
#include <functional>
#include "base/timestamp.h"

namespace ac_muduo::net {

    class timer_t {

    public:
        using timer_callback_t = std::function<void()>;

        timer_t(timer_callback_t cb, timestamp when, double interval) :
                callback_(std::move(cb)),
                expiration_(when),
                interval_(interval),
                repeat_(interval > .0),
                sequence_(++s_num_created_) {

        }

        void run() const {
            this->callback_();
        }

        timestamp expiration() const {
            return this->expiration_;
        }

        bool repeat() const {
            return this->repeat_;
        }

        int64_t sequence() const {
            return this->sequence_;
        }

        void restart(timestamp now);

        static int num_created() {
            return s_num_created_;
        }

    private:

        const timer_callback_t callback_;
        timestamp expiration_;

        const double interval_;
        const bool repeat_;
        const int64_t sequence_;
        static std::atomic_int64_t s_num_created_;
    };

}

#endif //AC_MUDUO_TIMER_T_H
