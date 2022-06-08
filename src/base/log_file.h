//
// Created by acai on 6/8/22.
//

#ifndef AC_MUDUO_LOG_FILE_H
#define AC_MUDUO_LOG_FILE_H

#include "types.h"
#include "noncopyable.h"
#include <memory>

namespace ac_muduo {
    namespace file_utils {
        class append_file;
    }
    class log_file : public noncopyable {

    public:
        log_file(const string &basename,
                 off_t roll_size,
                 bool thread_safe = true,
                 int flush_interval = 3,
                 int check_every_n = 1024);

        ~log_file();

        void append(const char *logline, int len);

        void flush();

        bool roll_file();

    private:
        void append_unlocked(const char *log_line, int len);

        static string get_log_filename(const string &basename, time_t *now);

        const string basename_;
        const off_t roll_size_;
        const int flush_interval_;
        const int check_every_n_;

        int count_;

        std::unique_ptr<file_utils::append_file> file_;
        std::unique_ptr<std::mutex> mutex_;

        time_t start_of_period_;
        time_t last_roll_;
        time_t last_flsh_;

        const static int k_roll_pre_seconds = 60 * 60 * 24;

    };

}

#endif //AC_MUDUO_LOG_FILE_H
