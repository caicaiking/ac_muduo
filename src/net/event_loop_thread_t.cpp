//
// Created by acai on 6/12/22.
//

#include "event_loop_thread_t.h"
#include "event_loop_t.h"
#include <mutex>

using namespace ac_muduo::net;

event_loop_thread_t::event_loop_thread_t(const event_loop_thread_t::thread_init_callback_t &cb,
                                         const std::string &name) :
        loop_(nullptr),
        exiting_(false),
        thread_([&] { this->thread_func(); }),
        callback_(cb) {

}

event_loop_thread_t::~event_loop_thread_t() {

    this->exiting_ = true;
    if (this->loop_ != nullptr) {
        this->loop_->quit();
        this->thread_.join();
    }
}

event_loop_t *event_loop_thread_t::start_loop() {
    assert(!this->thread_.started());
    this->thread_.start();

    event_loop_t *loop = nullptr;
    {

        std::unique_lock<std::mutex> lock(this->mutex_);
        while (this->loop_ == nullptr) {
            this->cond_.wait(lock);
        }

        loop = this->loop_;
    }

    return loop;
}

void event_loop_thread_t::thread_func() {
    event_loop_t loop;
    if (this->callback_) {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> lock(this->mutex_);
        this->loop_ = &loop;
        this->cond_.notify_one();
    }

    loop.loop();
    std::lock_guard<std::mutex> lock(this->mutex_);
    this->loop_ = nullptr;
}
