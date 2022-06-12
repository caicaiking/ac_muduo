//
// Created by acai on 6/12/22.
//

#include "event_loop_thead_pool_t.h"
#include "event_loop_t.h"
#include "event_loop_thread_t.h"

namespace ac_muduo::net {


    event_loop_thead_pool_t::event_loop_thead_pool_t(event_loop_t *base_loop,
                                                     const string &name_arg)
            : base_loop_(base_loop),
              name_(name_arg),
              started_(false),
              num_threads_(0),
              next_(0) {

    }

    event_loop_thead_pool_t::~event_loop_thead_pool_t() {
        // Don't delete loop, it's stack variable
    }

    void event_loop_thead_pool_t::start(const event_loop_thead_pool_t::thread_init_callback_t &cb) {
        assert(!this->started_);
        this->base_loop_->assert_in_loop_thread();
        this->started_ = true;

        for (int i = 0; i < num_threads_; ++i) {
            char buf[name_.size() + 32];
            snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);

            event_loop_thread_t *t = new event_loop_thread_t(cb, buf);
            threads_.push_back(std::unique_ptr<event_loop_thread_t>(t));
            this->loops_.push_back(t->start_loop());
        }
        if (this->num_threads_ == 0 && cb) {
            cb(this->base_loop_);
        }
    }

    event_loop_t *event_loop_thead_pool_t::get_next_loop() {
        this->base_loop_->assert_in_loop_thread();
        assert(this->started_);

        event_loop_t *loop = this->base_loop_;
        if (!loops_.empty()) {
            loop = this->loops_[next_];
            ++next_;
            if (implicit_cat<size_t>(this->next_) >= this->loops_.size()) {
                this->next_ = 0;
            }
        }
        return loop;
    }

    event_loop_t *event_loop_thead_pool_t::get_loop_for_hash(size_t hash_code) {
        this->base_loop_->assert_in_loop_thread();
        event_loop_t *loop = this->base_loop_;

        if (!this->loops_.empty()) {
            loop = this->loops_[hash_code % loops_.size()];
        }
        return loop;
    }

    std::vector<event_loop_t *> event_loop_thead_pool_t::get_all_loops() {
        this->base_loop_->assert_in_loop_thread();
        assert(this->started_);
        if (this->loops_.empty()) {
            return std::vector<event_loop_t *>(1, this->base_loop_);
        } else {
            return this->loops_;
        }
    }
}