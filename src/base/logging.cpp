//
// Created by acai on 6/7/22.
//

#include "logging.h"
#include "current_thead.h"
#include "timestamp.h"
#include "time_zone.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>


namespace ac_muduo {

    __thread char t_errno_buf[512];
    __thread char t_time[64];
    __thread time_t t_last_second;

    const char *strerror_tl(int save_errno) {
        return strerror_r(save_errno, t_errno_buf, sizeof(t_errno_buf));
    }

    logger::log_level_t init_log_level() {
        if (::getenv("AC_MUDUO_LOG_TRACE")) {
            return logger::log_level_t::TRACE;
        } else if (::getenv("AC_MUDUO_LOG_DEBUG")) {
            return logger::log_level_t::DEBUG;
        } else {
            return logger::log_level_t::INFO;
        }
    }

    logger::log_level_t g_log_level = init_log_level();


    const char *log_level_name[logger::log_level_t::NUM_LOG_LEVELS] =
            {
                    "TRACE ",
                    "DEBUG ",
                    "INFO  ",
                    "WARN  ",
                    "ERROR ",
                    "FATAL ",
            };

    class T {
    public:
        T(const char *str, unsigned len) : str_(str), len_(len) {
            assert(strlen(str) == len_);
        }

        const char *str_;
        const unsigned len_;
    };


    inline log_stream &operator<<(log_stream &s, T v) {
        s.append(v.str_, v.len_);
        return s;
    }

    inline log_stream &operator<<(log_stream &s, const logger::source_file &v) {
        s.append(v.data_, v.size_);
        return s;
    }

    void default_output(const char *msg, int len) {
        size_t n = fwrite(msg, 1, len, stdout);
        (void) n;
    }

    void default_flush() {
        fflush(stdout);
    }

    logger::output_func g_output = default_output;
    logger::flush_func g_flush = default_flush;
    time_zone g_log_time_zone;


}

namespace ac_muduo {

    logger::impl::impl(log_level
                       level,
                       int old_errno,
                       const source_file &file,
                       int line
    ) :

            time_(timestamp::now()),
            stream_(),
            level_(level),
            line_(line),
            base_name_(file) {
        this->format_time();
        current_thread::tid();
        stream_ << T(current_thread::tid_string(), current_thread::tid_string_length());
        stream_ << T(log_level_name[static_cast<int>(level)], 6);
        if (old_errno != 0) {
            stream_ << strerror_tl(old_errno) << " (error=" << old_errno << ") ";
        }
    }

    void logger::impl::format_time() {
        int64_t microSecondsSinceEpoch = time_.micro_seconds_since_epoch();
        time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / timestamp::k_micro_seconds_per_second);
        int microseconds = static_cast<int>(microSecondsSinceEpoch % timestamp::k_micro_seconds_per_second);
        if (seconds != t_last_second) {
            t_last_second = seconds;
            struct tm tm_time;
            if (g_log_time_zone.valid()) {
                tm_time = g_log_time_zone.to_local_time(seconds);
            } else {
                ::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime
            }

            int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                               tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                               tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
            assert(len == 17);
            (void) len;
        }

        if (g_log_time_zone.valid()) {
            fmt us(".%06d ", microseconds);
            assert(us.length() == 8);
            stream_ << T(t_time, 17) << T(us.data(), 8);
        } else {
            fmt us(".%06dZ ", microseconds);
            assert(us.length() == 9);
            stream_ << T(t_time, 17) << T(us.data(), 9);
        }
    }

    void logger::impl::finish() {
        this->stream_ << " - " << this->base_name_ << ":" << line_ << '\n';
    }


    logger::logger(source_file file, int line)
            : impl_(logger::log_level_t::INFO,
                    0,
                    file,
                    line) {

    }

    logger::logger(source_file file, int line, log_level_t level)
            : impl_(level,
                    0,
                    file,
                    line) {

    }

    logger::logger(source_file file, int line, log_level_t level, const char *func)
            : impl_(level,
                    0,
                    file,
                    line) {
        impl_.stream_ << func << " ";
    }

    logger::logger(source_file file, int line, bool to_abort)
            : impl_(to_abort ? logger::log_level_t::FATAL : logger::log_level_t::ERROR,
                    errno,
                    file,
                    line) {

    }

    logger::~logger() {
        this->impl_.finish();
        const log_stream::buffer_t &buf(this->stream().buffer());
        g_output(buf.data(), buf.length());
        if (impl_.level_ == logger::log_level_t::FATAL) {
            g_flush();
            abort();
        }

    }

    void logger::set_log_level(log_level_t level) {
        g_log_level = level;

    }

    void logger::set_output_fun(output_func out) {
        g_output = out;
    }

    void logger::set_flush(flush_func flush) {
        g_flush = flush;
    }

    void logger::set_time_zone(const time_zone &tz) {
        g_log_time_zone = tz;
    }

    void logger::flush() {
        g_flush();
    }


}
