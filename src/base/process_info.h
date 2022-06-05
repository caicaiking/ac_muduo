//
// Created by acai on 6/4/22.
//

#ifndef AC_MUDUO_PROCESS_INFO_H
#define AC_MUDUO_PROCESS_INFO_H

#include "types.h"
#include <sys/types.h>
#include "string_piece.h"
#include <vector>
#include "timestamp.h"

namespace ac_muduo {
    namespace process_info {

        pid_t pid();

        string pid_string();

        uid_t uid();

        string user_name();

        uid_t euid();

        timestamp start_time();

        int clock_ticks_per_seconds();

        int page_size();

        bool is_debug_build();

        string host_name();
        string proc_name();

        string_piece proc_name(const string& stat);

        string proc_status();

        string proc_stat();

        string exe_path();

        string thread_stat();

        int opened_files();

        int max_open_files();

        struct cpu_time_t
        {
            double user_seconds;
            double system_seconds;

            cpu_time_t(): user_seconds(.0), system_seconds(0.0){}
            double total() const
            {
                return this->user_seconds + this->system_seconds;
            }
        };

        cpu_time_t cpu_time();

        int num_thread();
        std::vector<pid_t> threads();
    }

}

#endif //AC_MUDUO_PROCESS_INFO_H
