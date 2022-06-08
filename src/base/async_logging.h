//
// Created by acai on 6/8/22.
//

#ifndef AC_MUDUO_ASYNC_LOGGING_H
#define AC_MUDUO_ASYNC_LOGGING_H

#include "blocking_queue.h"
#include "bounded_blocking_queue.h"
#include "count_down_latch.h"
#include "thread.h"
#include "log_stream.h"
#include <atomic>
#include <vector>

namespace ac_muduo {

    class async_logging : public noncopyable {
    public:
        async_logging(const string &basename,
                      off_t roll_size,
                      int flush_interval = 3
        );

        ~async_logging() {
            if (this->running_) {
                this->stop();
            }
        }

        void append(const char *logline, int len);

        void start() {
            this->running_ = true;
            this->thread_.start();
            latch_.wait();
        }

        void stop() {
            this->running_ = false;
            this->cond_.notify_one();
            thread_.join();
        }

    private:
        void thread_func();

        typedef ac_muduo::detail::fixed_buffer<ac_muduo::detail::k_large_buffer> buffer_t;
        typedef std::vector<std::unique_ptr<buffer_t >> buffer_vector_t;
        typedef buffer_vector_t::value_type buffer_ptr_t;

        const int flush_interval_;
        std::atomic_bool running_;
        const string basename_;
        const off_t roll_size_;
        ac_muduo::thread thread_;
        ac_muduo::count_down_latch latch_;
        std::mutex mutex_;
        std::condition_variable cond_;

        buffer_ptr_t current_buffer_;
        buffer_ptr_t next_buffer_;
        buffer_vector_t buffers_;
    };

}

#endif //AC_MUDUO_ASYNC_LOGGING_H
