//
// Created by acai on 6/7/22.
//

#ifndef AC_MUDUO_LOGGING_H
#define AC_MUDUO_LOGGING_H

#include "log_stream.h"
#include "timestamp.h"

namespace ac_muduo {
    class time_zone;

    class logger {
    public:
        enum log_level_t {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        class source_file {
        public:
            template<int N>
            source_file(const char(&arr)[N]): data_(arr), size_(N - 1) {
            }

            explicit source_file(const char *filename) : data_(filename) {
                this->size_ = static_cast<int>(strlen(this->data_));
            }

        public:
            const char *data_;
            int size_;
        };

        logger(source_file file, int line);

        logger(source_file file, int line, log_level_t level);

        logger(source_file file, int line, log_level_t level, const char *func);

        logger(source_file file, int line, bool to_abort);

        ~logger();

        log_stream &stream() { return this->impl_.stream_; }

        static log_level_t log_level();

        static void set_log_level(log_level_t level);

        typedef void(*output_func)(const char *msg, int len);

        typedef void(*flush_func)();

        static void set_output_fun(output_func);

        static void set_flush(flush_func);

        static void set_time_zone(const time_zone &tz);

        static void flush();

    private:
        class impl {
        public:
            typedef logger::log_level_t log_level;

            impl(log_level level, int old_errno, const source_file &file, int line);

            void format_time();

            void finish();

            timestamp time_;
            log_stream stream_;
            log_level level_;
            int line_;
            source_file base_name_;
        };

        impl impl_;
    };

    extern logger::log_level_t g_log_level;

    inline logger::log_level_t logger::log_level() {
        return g_log_level;
    }

    const char *strerror_tl(int save_errno);

#define CHECK_NOTNULL(val) \
::ac_muduo::check_not_null(__FILE__, __LINE__, "'" #val "' Must bu non nullptr", (val))

    template<typename T>
    T *check_not_null(logger::source_file file, int line, const char *names, T *ptr) {
        if (ptr == nullptr) {
            logger(file, line, logger::log_level_t::FATAL).stream() << names;
        }
        return ptr;
    }

#define LOG_TRACE if (ac_muduo::logger::log_level() <= ac_muduo::logger::TRACE) \
  ac_muduo::logger(__FILE__, __LINE__, ac_muduo::logger::log_level_t::TRACE, __func__).stream()
#define LOG_DEBUG if (ac_muduo::logger::log_level() <= ac_muduo::logger::log_level_t::DEBUG) \
  ac_muduo::logger(__FILE__, __LINE__, ac_muduo::logger::DEBUG, __func__).stream()
#define LOG_INFO if (ac_muduo::logger::log_level() <= ac_muduo::logger::log_level_t::INFO) \
  ac_muduo::logger(__FILE__, __LINE__).stream()
#define LOG_WARN ac_muduo::logger(__FILE__, __LINE__, ac_muduo::logger::log_level_t::WARN).stream()
#define LOG_ERROR ac_muduo::logger(__FILE__, __LINE__, ac_muduo::logger::log_level_t::ERROR).stream()
#define LOG_FATAL ac_muduo::logger(__FILE__, __LINE__, ac_muduo::logger::log_level_t::FATAL).stream()
#define LOG_SYSERR ac_muduo::logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL ac_muduo::logger(__FILE__, __LINE__, true).stream()

}


#endif //AC_MUDUO_LOGGING_H
