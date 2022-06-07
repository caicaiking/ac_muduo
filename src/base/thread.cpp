//
// Created by acai on 5/22/22.
//

#include "thread.h"
#include <unistd.h>
#include <sys/syscall.h>
#include "current_thead.h"
#include <sys/prctl.h>
#include <cassert>
#include "exception.h"


namespace ac_muduo {
    namespace detail {
        pid_t get_tid() {
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }

    void current_thread::cache_tid() {
        if (t_cached_tid_ == 0) {
            t_cached_tid_ = detail::get_tid();
            t_tid_string_length = snprintf(t_tid_string, sizeof t_tid_string, "%5d ", t_cached_tid_);
        }
    }

    bool current_thread::is_main_thread() {
        return tid() == ::getpid();
    }

    std::atomic_int thread::num_created_ = {};

    thread::thread(thread::thread_func func, const string &name) :
            started_(false),
            joined_(false),
            func_(std::move(func)),
            name_(name),
            latch_(1) {
        this->set_default_name();

    }

    thread::~thread() {
        if (this->started_ && !this->joined_) {
            this->thread_.detach();
        }
    }

    void thread::start() {
        assert(!this->started_);
        this->started_ = true;
        this->thread_ = std::move(std::thread([this] {
                                                  this->latch_.count_down();

                                                  current_thread::t_thread_name_ =
                                                          name_.empty() ?
                                                          "muduoThead" : name_.c_str();
                                                  ::prctl(PR_SET_NAME, current_thread::t_thread_name_);

                                                  try {
                                                      func_();
                                                      current_thread::t_thread_name_ = "finished";
                                                  }
                                                  catch (const exception &ex) {
                                                      current_thread::t_thread_name_ = "crashed";
                                                      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                                                      fprintf(stderr, "reason: %s\n", ex.what());
                                                      fprintf(stderr, "stack trace: %s\n", ex.stack_trace());
                                                      abort();
                                                  }
                                                  catch (const std::exception &ex) {
                                                      current_thread::t_thread_name_ = "crashed";
                                                      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                                                      fprintf(stderr, "reason: %s\n", ex.what());
                                                      abort();
                                                  }
                                                  catch (...) {
                                                      current_thread::t_thread_name_ = "crashed";
                                                      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
                                                      throw; // rethrow
                                                  }
                                              }
                                  )
        );
        this->latch_.wait();
    }

    void thread::join() {
        assert(started_);
        assert(!joined_);
        this->joined_ = true;

        if (this->thread_.joinable()) {
            this->thread_.join();
        }
    }

    void thread::set_default_name() {
        num_created_++;
        if (this->name_.empty()) {
            char buf[32];
            snprintf(buf, sizeof buf, "thread%d", num_created_.load());
            this->name_ = buf;
        }
    }
}
