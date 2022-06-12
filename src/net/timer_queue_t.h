//
// Created by acai on 6/11/22.
//

#ifndef AC_MUDUO_TIMER_QUEUE_T_H
#define AC_MUDUO_TIMER_QUEUE_T_H

#include "base/noncopyable.h"
#include "timer_id_t.h"
#include <set>
#include "channel_t.h"
#include <vector>

namespace ac_muduo::net {
    class event_loop_t;

    class timer_queue_t : public noncopyable {
    public:
        using timer_callback_t = std::function<void()>;

        explicit timer_queue_t(event_loop_t *loop);

        ~timer_queue_t();

        timer_id_t add_timer(timer_callback_t cb, timestamp when, double interval);

        void cancel(timer_id_t timer_id);

    private:

        using entry_t = std::pair<timestamp, timer_t *>;
        using timer_list_t = std::set<entry_t>;

        using active_timer_t = std::pair<timer_t *, int64_t>;
        using active_timer_set_t = std::set<active_timer_t>;

        void add_timer_in_loop(timer_t *timer);

        void cancel_in_loop(timer_id_t timer_id);

        void handle_read();

        std::vector<entry_t> get_expired(timestamp now);

        void reset(const std::vector<entry_t> &expired, timestamp now);

        bool insert(timer_t *timer);

    private:
        event_loop_t *loop_;
        const int timer_fd_;

        channel_t timer_fd_channel_;
        timer_list_t timers_;

        active_timer_set_t active_timers_;
        bool calling_expired_timers_;
        active_timer_set_t canceling_timers_;
    };
}

#endif //AC_MUDUO_TIMER_QUEUE_T_H
