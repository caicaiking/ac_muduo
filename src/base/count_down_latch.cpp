//
// Created by acai on 5/22/22.
//

#include "count_down_latch.h"

namespace ac_muduo {

    count_down_latch::count_down_latch(int count) :
            count_(count) {
    }

    void count_down_latch::wait() {
        std::unique_lock<std::mutex> lk(this->mutex_);
        while (this->count_ > 0) {
            this->condition_.wait(lk);
        }
    }

    void count_down_latch::count_down() {
        std::lock_guard<std::mutex> lk(mutex_);
        --count_;

        if (count_ == 0) {
            this->condition_.notify_all();
        }
    }

    int count_down_latch::get_count() const {
        std::lock_guard<std::mutex> lk(mutex_);
        return this->count_;
    }
}