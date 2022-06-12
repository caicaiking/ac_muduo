//
// Created by acai on 6/12/22.
//

#ifndef AC_MUDUO_EVENT_LOOP_THEAD_POOL_T_H
#define AC_MUDUO_EVENT_LOOP_THEAD_POOL_T_H

#include "base/noncopyable.h"
#include "event_loop_t.h"
#include <functional>
#include <memory>
#include <vector>
#include "base/types.h"


namespace ac_muduo::net {

    class event_loop_t;

    class event_loop_thread_t;

    class event_loop_thead_pool_t : public noncopyable {

    public:
        using thread_init_callback_t = std::function<void(event_loop_t *)>;

        event_loop_thead_pool_t(event_loop_t *base_loop, const string &name_arg);

        ~event_loop_thead_pool_t();

        void set_thread_num(int num_threads) {
            this->num_threads_ = num_threads;
        }

        void start(const thread_init_callback_t &cb = thread_init_callback_t());

        event_loop_t *get_next_loop();

        event_loop_t *get_loop_for_hash(size_t hash_code);

        std::vector<event_loop_t *> get_all_loops();


        bool started() const {
            return this->started_;
        }

        const string &name() const {
            return this->name_;
        }


    private:
        event_loop_t *base_loop_;
        string name_;
        bool started_;
        int num_threads_;
        int next_;
        std::vector<std::unique_ptr<event_loop_thread_t>> threads_;
        std::vector<event_loop_t *> loops_;

    };

}

#endif //AC_MUDUO_EVENT_LOOP_THEAD_POOL_T_H
