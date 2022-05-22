//
// Created by acai on 5/22/22.
//

#ifndef AC_MUDUO_COUNT_DOWN_LATCH_H
#define AC_MUDUO_COUNT_DOWN_LATCH_H

#include <mutex>
#include <condition_variable>

namespace ac_muduo {

    class count_down_latch {
    public:
        explicit count_down_latch(int count);

        void wait();

        void count_down();

        int get_count() const;

    private:
        mutable std::mutex mutex_;
        std::condition_variable condition_;
        int count_;
    };

}

#endif //AC_MUDUO_COUNT_DOWN_LATCH_H
