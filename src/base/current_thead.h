//
// Created by acai on 5/22/22.
//

#ifndef AC_MUDUO_CURRENT_THEAD_H
#define AC_MUDUO_CURRENT_THEAD_H

#include <chrono>
#include "types.h"

namespace ac_muduo {
    namespace current_thread {
        extern __thread int t_cached_tid_;
        extern __thread const char *t_thread_name_;
        extern __thread char t_tid_string[32];
        extern __thread int t_tid_string_length;

        void cache_tid();

        inline int tid() {
            if (t_cached_tid_ == 0) {
                cache_tid();
            }

            return t_cached_tid_;
        }

        inline const char *tid_string() {
            return t_tid_string;
        }

        inline int tid_string_length() {
            return t_tid_string_length;
        }

        bool is_main_thread();

        inline const char *name() {
            return t_thread_name_;
        }

        void sleep(std::chrono::microseconds s);

        string stack_trace(bool damangle);
    }
}


#endif //AC_MUDUO_CURRENT_THEAD_H
