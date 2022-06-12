//
// Created by acai on 6/9/22.
//

#ifndef AC_MUDUO_EVENT_LOOP_T_H
#define AC_MUDUO_EVENT_LOOP_T_H

#include <memory>
#include <atomic>
#include <functional>
#include <boost/any.hpp>
#include "base/current_thead.h"
#include "base/timestamp.h"
#include "base/noncopyable.h"
#include "timer_id_t.h"

namespace ac_muduo::net {
    class channel_t;

    class poller_t;

    class timer_queue_t;

    class event_loop_t : public noncopyable {

    public:
        using functor_t = std::function<void()>;

        event_loop_t();

        ~event_loop_t();

        void loop();

        void quit();

        timestamp poll_return_time() const {
            return this->poll_return_time_;
        }

        int64_t iteration() const {
            return this->iteration_;
        }

        void run_in_loop(functor_t cb);

        void queue_in_loop(functor_t cb);

        size_t queue_size() const;

        timer_id_t run_at(timestamp time, std::function<void()> cb);

        timer_id_t run_after(double delay, std::function<void()> cb);

        timer_id_t run_every(double delay, std::function<void()> cb);

        void cancel(timer_id_t timer_id);

        void wakeup();

        void update_channel(channel_t *channel);

        void remove_channel(channel_t *channel);

        bool has_channel(channel_t *pChannel);

        void assert_in_loop_thread() {
            if (!this->is_in_loop_thread()) {
                this->abort_not_in_loop_thread();
            }
        }

        bool is_in_loop_thread() {
            return this->thread_id_ == current_thread::tid();
        }

        bool event_handling() const {
            return this->event_handling_;
        }

        void set_context(const boost::any &context) {
            this->context_ = context;
        }

        const boost::any &get_context() const {
            return this->context_;
        }

        boost::any *get_mutable_context() {
            return &context_;
        }

        static event_loop_t *get_event_loop_of_current_thread();

    private:
        void abort_not_in_loop_thread();

        void handle_read();

        void do_pending_functors();

        void print_active_channels() const;

        using channel_list_t = std::vector<channel_t *>;

        std::atomic_bool looping_;
        std::atomic_bool quit_;
        std::atomic_bool event_handling_;
        std::atomic_bool calling_pending_functors_;

        int64_t iteration_;
        const pid_t thread_id_;
        timestamp poll_return_time_;
        std::unique_ptr<poller_t> poller_;
        std::unique_ptr<timer_queue_t> timer_queue_;

        int wakeup_fd_;
        std::unique_ptr<channel_t> wakeup_channel_;
        boost::any context_;

        channel_list_t active_channels_;
        channel_t *current_active_channel_;

        mutable std::mutex mutex_;
        std::vector<functor_t> pending_functors_;

    };

}

#endif //AC_MUDUO_EVENT_LOOP_T_H
