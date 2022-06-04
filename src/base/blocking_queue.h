//
// Created by acai on 6/3/22.
//

#ifndef AC_MUDUO_BLOCKING_QUEUE_H
#define AC_MUDUO_BLOCKING_QUEUE_H

#include "noncopyable.h"
#include <mutex>
#include <condition_variable>
#include <deque>

namespace ac_muduo {
    template<class T>
    class blocking_queue : public noncopyable {
    public:
        using queue_type = std::deque<T>;

        blocking_queue() : queue_() {}

        void put(const T &x) {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->queue_.push_back(x);
            this->not_empty_.notify_one();
        }

        void put(T &&x) {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->queue_.push_back(std::move(x));
            this->not_empty_.notify_one();
        }

        T take() {
            std::unique_lock<std::mutex> lock(this->mutex_);
            while (this->queue_.empty()) {
                this->not_empty_.wait(lock);
            }
            assert(!this->queue_.empty());

            T front(this->queue_.front());
            this->queue_.pop_front();
            return front;
        }

        queue_type drain() {
            queue_type queue;
            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                queue = std::move(queue_);
                assert(this->queue_.empty());
            }
            return queue;
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(this->mutex_);
            return this->queue_.size();
        }

    private:
        std::mutex mutex_;
        std::condition_variable not_empty_;
        queue_type queue_;


    };


}
#endif //AC_MUDUO_BLOCKING_QUEUE_H
