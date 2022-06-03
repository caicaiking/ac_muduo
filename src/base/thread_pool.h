#pragma once

#include "types.h"

#include <deque>
#include <vector>
#include "noncopyable.h"
#include <functional>
#include <mutex>
#include <condition_variable>
#include "thread.h"

namespace ac_muduo
{
    class thread_pool : public noncopyable
    {
        public:
            using task_t = std::function<void()>;
            explicit thread_pool(const string& name_arg = string("thread_pool"));
            ~thread_pool();

            void set_max_queue_size(int max_size){this->max_queue_size_ = max_size;}
            void set_thread_init_callback(const task_t& cb)
            {
                this->thread_init_callback_ = cb;
            }

            void start(int num_threads);
            void stop();

            const string& name() const
            {
                return this->name_;
            }

            size_t queue_size() const;
            void run (task_t f);

        private:
            bool is_full() const;
            void run_in_thread();
            task_t take();

            mutable std::mutex mutex_;
            std::condition_variable not_empty_;
            std::condition_variable not_full_;

            string name_;
            task_t thread_init_callback_;
            std::vector<std::unique_ptr<thread>> threads_;
            std::deque<task_t> queue_;
            size_t max_queue_size_;
            bool running_;
    };
}
