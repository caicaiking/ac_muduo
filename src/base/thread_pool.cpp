#include "thread_pool.h"
#include <cassert>
#include "exception.h"
#include <iostream>

namespace ac_muduo {

    thread_pool::thread_pool(const string &name_arg) :
            name_(name_arg), max_queue_size_(0), running_(false) {
    }

    thread_pool::~thread_pool() {
        if (this->running_) {
            this->stop();
        }
    }

    void thread_pool::start(int num_threads) {
        assert(this->threads_.empty());
        this->running_ = true;
        this->threads_.reserve(num_threads);

        for (int i = 0; i < num_threads; ++i) {
            char id[32];
            snprintf(id, sizeof id, "%d", i + 1);
            this->threads_.emplace_back(
                    new ac_muduo::thread(std::bind(&thread_pool::run_in_thread, this), this->name_ + id));
            this->threads_[i]->start();
        }

        if (num_threads == 0 && this->thread_init_callback_) {
            this->thread_init_callback_();
        }
    }

    void thread_pool::stop() {
        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->running_ = false;
            this->not_empty_.notify_all();
            this->not_full_.notify_all();
        }

        for (auto &thr: this->threads_) {
            thr->join();
        }
    }

    size_t thread_pool::queue_size() const {
        std::lock_guard<std::mutex> lock(this->mutex_);
        return this->queue_.size();
    }

    void thread_pool::run(task_t f) {
        if (this->threads_.empty()) {
            f();
        } else {
            std::unique_lock<std::mutex> lock(this->mutex_);
            while (this->is_full() && this->running_) {
                this->not_full_.wait(lock);
            }
            if (!this->running_) return;
            assert(!this->is_full());

            this->queue_.push_back(std::move(f));
            this->not_empty_.notify_one();
        }
    }

    thread_pool::task_t thread_pool::take() {
        std::unique_lock<std::mutex> lock(this->mutex_);

        while (this->queue_.empty() && this->running_) {
            this->not_empty_.wait(lock);
        }

        thread_pool::task_t task;
        if (!this->queue_.empty()) {
            task = this->queue_.front();
            this->queue_.pop_front();

            if (this->max_queue_size_ > 0) {
                this->not_full_.notify_one();
            }

        }
        return task; //this task could be nullptr;
    }

    bool thread_pool::is_full() const {
        return this->max_queue_size_ > 0 && queue_.size() >= this->max_queue_size_;
    }

    void thread_pool::run_in_thread() {
        try {
            if (this->thread_init_callback_) {
                this->thread_init_callback_();
            }

            while (this->running_) {
                task_t task(this->take());
                if (task) {
                    task();
                }
            }
        }
        catch (const exception &ex) {
            fprintf(stderr, "exception caught in thread_pool %s\n", this->name_.data());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stack_trace());
            abort();
        }
        catch (const std::exception &ex) {
            fprintf(stderr, "exception caught in thread_pool %s\n", this->name_.data());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...) {
            fprintf(stderr, "unknown exception caught in thread_pool %s", name_.data());
            throw;
        }
    }
}
