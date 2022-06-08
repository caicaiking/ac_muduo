//
// Created by acai on 6/8/22.
//

#include "log_file.h"
#include "file_utils.h"
#include "process_info.h"
#include <iostream>

#include <cassert>
#include <stdio.h>
#include <time.h>


namespace ac_muduo {

    log_file::log_file(const std::string &basename,
                       off_t roll_size,
                       bool thread_safe,
                       int flush_interval,
                       int check_every_n)
            : basename_(basename),
              roll_size_(roll_size),
              flush_interval_(flush_interval),
              check_every_n_(check_every_n),
              count_(0),
              mutex_(thread_safe ? new std::mutex : nullptr),
              start_of_period_(0),
              last_roll_(0),
              last_flsh_(0) {
        assert(basename.find('/') == string::npos);
        this->roll_file();
    }

    log_file::~log_file() = default;


    void log_file::append(const char *logline, int len) {
        if (this->mutex_) {
            std::lock_guard<std::mutex> lock(*this->mutex_);
            append_unlocked(logline, len);
        } else {
            append_unlocked(logline, len);
        }

    }

    void log_file::flush() {
        if (this->mutex_) {
            std::lock_guard<std::mutex> lock(*this->mutex_);
            this->file_->flush();
        } else {
            this->file_->flush();
        }
    }

    bool log_file::roll_file() {

        time_t now = 0;
        string filename = get_log_filename(this->basename_, &now);
        time_t start = now / this->k_roll_pre_seconds * this->k_roll_pre_seconds;
        if (now > last_roll_) {
            last_roll_ = now;
            last_flsh_ = now;
            start_of_period_ = start;
            this->file_.reset(new file_utils::append_file(filename));
            return true;
        }

        return false;
    }

    void log_file::append_unlocked(const char *log_line, int len) {
        this->file_->append(log_line, len);

        if (this->file_->written_bytes() > roll_size_) {
            this->roll_file();
        } else {
            ++count_;
            if (count_ >= check_every_n_) {
                this->count_ = 0;
                time_t now = time(nullptr);
                time_t this_period = now / this->k_roll_pre_seconds * this->k_roll_pre_seconds;
                if (this_period != start_of_period_) {
                    roll_file();
                } else if (now - last_flsh_ > this->flush_interval_) {
                    last_flsh_ = now;
                    this->file_->flush();
                }
            }
        }
    }

    string log_file::get_log_filename(const std::string &basename, time_t *now) {
        string filename;
        filename.reserve(basename.size() + 64);
        filename = basename;

        char timebuf[32];
        struct tm tm;
        *now = time(nullptr);
        gmtime_r(now, &tm);

        strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
        filename += timebuf;
        filename += process_info::host_name();

        char pidbuf[32];
        snprintf(pidbuf, sizeof pidbuf, ".%d", process_info::pid());
        filename += pidbuf;
        filename += ".log";
        return filename;
    }

}
