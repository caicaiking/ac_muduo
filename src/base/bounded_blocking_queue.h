//
// Created by acai on 6/4/22.
//

#ifndef AC_MUDUO_BOUNDED_BLOCKING_QUEUE_H
#define AC_MUDUO_BOUNDED_BLOCKING_QUEUE_H

#include "noncopyable.h"
#include <mutex>
#include <condition_variable>
#include <boost/circular_buffer.hpp>
#include <iostream>

namespace ac_muduo {
    template<class T>
    class bounded_blocking_queue : public noncopyable {
    public:
        explicit bounded_blocking_queue(int max_size) : queue_(max_size) {}

        void put(const T &x) {
            std::unique_lock<std::mutex> lock(this->mutex_);

            while (this->queue_.full()) {
                std::cout << "full" << std::endl;
                this->not_full_.wait(lock);
            }
            assert(!this->queue_.full());
            this->queue_.push_back(x);
            this->not_empty_.notify_one();
        }

        void put(T &&x) {
            std::unique_lock<std::mutex> lock(this->mutex_);

            while (this->queue_.full()) {
                std::cout << "full" << std::endl;
                this->not_full_.wait(lock);
            }
            assert(!this->queue_.full());
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
            this->not_full_.notify_one();
            return front;
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(this->mutex_);
            return this->queue_.empty();
        }

        bool full() const {
            std::lock_guard<std::mutex> lock(this->mutex_);
            return this->queue_.full();
        }

        bool size() const {
            std::lock_guard<std::mutex> lock(this->mutex_);
            return this->queue_.size();
        }

        bool capacity() const {
            std::lock_guard<std::mutex> lock(this->mutex_);
            return this->queue_.capacity();
        }

    private:
        mutable std::mutex mutex_;
        std::condition_variable not_empty_;
        std::condition_variable not_full_;
        boost::circular_buffer<T> queue_;

    };


}
#endif //AC_MUDUO_BOUNDED_BLOCKING_QUEUE_H
