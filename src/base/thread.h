//
// Created by acai on 5/22/22.
//

#ifndef AC_MUDUO_THREAD_H
#define AC_MUDUO_THREAD_H

#include "noncopyable.h"
#include <functional>
#include <thread>
#include <atomic>
#include "count_down_latch.h"
#include "types.h"

namespace ac_muduo {

    class thread : public noncopyable {
    public:
        using thread_func = std::function<void()>;

        explicit thread(thread_func func, const string &name = string());

        ~thread();

        void start();

        void join();

        bool started() const { return this->started_; }

        const std::string &name() const { return this->name_; }

        static int num_created() { return num_created_; }

    private:

        void set_default_name();

        bool started_;
        bool joined_;
        std::thread thread_;
        thread_func func_;
        std::string name_;
        count_down_latch latch_;
        static std::atomic_int num_created_;

    };

}

#endif //AC_MUDUO_THREAD_H
