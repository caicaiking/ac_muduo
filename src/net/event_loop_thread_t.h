//
// Created by acai on 6/12/22.
//

#ifndef AC_MUDUO_EVENT_LOOP_THREAD_T_H
#define AC_MUDUO_EVENT_LOOP_THREAD_T_H

#include "base/noncopyable.h"
#include "base/thread.h"
#include <mutex>
#include <condition_variable>


namespace ac_muduo::net {
    class event_loop_t;

    class event_loop_thread_t {
    public:
        using thread_init_callback_t = std::function<void(event_loop_t *)>;

        event_loop_thread_t(const thread_init_callback_t &cb = thread_init_callback_t(),
                            const string &name = string());

        ~event_loop_thread_t();

        event_loop_t *start_loop();

    private:
        void thread_func();

        event_loop_t *loop_;
        bool exiting_;
        thread thread_;
        std::mutex mutex_;
        std::condition_variable cond_;
        thread_init_callback_t callback_;

    };

}

#endif //AC_MUDUO_EVENT_LOOP_THREAD_T_H
