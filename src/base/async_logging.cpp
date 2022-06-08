//
// Created by acai on 6/8/22.
//


#include "async_logging.h"
#include "log_file.h"
#include "timestamp.h"

namespace ac_muduo {

    async_logging::async_logging(const std::string &basename,
                                 off_t roll_size,
                                 int flush_interval)
            : flush_interval_(flush_interval),
              running_(false),
              basename_(basename),
              roll_size_(roll_size),
              thread_(std::bind(&async_logging::thread_func, this), "logging"),
              latch_(1),
              current_buffer_(new buffer_t),
              next_buffer_(new buffer_t),
              buffers_() {
        this->current_buffer_->bzero();
        this->next_buffer_->bzero();
        this->buffers_.reserve(16);
    }

    void async_logging::append(const char *logline, int len) {
        std::lock_guard<std::mutex> lock(this->mutex_);
        if (this->current_buffer_->avail() > len) {
            this->current_buffer_->append(logline, len);
        } else {
            this->buffers_.push_back(std::move(this->current_buffer_));
            if (next_buffer_) {
                this->current_buffer_ = std::move(next_buffer_);
            } else {
                this->current_buffer_.reset(new buffer_t);
            }

            this->current_buffer_->append(logline, len);
            cond_.notify_one();
        }
    }

    void async_logging::thread_func() {
        assert(running_ == true);
        latch_.count_down();
        log_file output(basename_, this->roll_size_, false, 3, 10);
        buffer_ptr_t newBuffer1(new buffer_t);
        buffer_ptr_t newBuffer2(new buffer_t);
        newBuffer1->bzero();
        newBuffer2->bzero();
        buffer_vector_t buffersToWrite;
        buffersToWrite.reserve(16);
        while (running_) {
            assert(newBuffer1 && newBuffer1->length() == 0);
            assert(newBuffer2 && newBuffer2->length() == 0);
            assert(buffersToWrite.empty());

            {
                std::unique_lock<std::mutex> lock(this->mutex_);
                if (buffers_.empty())  // unusual usage!
                {
                    this->cond_.wait_for(lock, std::chrono::seconds(this->flush_interval_));
                }
                buffers_.push_back(std::move(this->current_buffer_));
                this->current_buffer_ = std::move(newBuffer1);
                buffersToWrite.swap(buffers_);
                if (!this->next_buffer_) {
                    this->next_buffer_ = std::move(newBuffer2);
                }
            }

            assert(!buffersToWrite.empty());

            if (buffersToWrite.size() > 25) {
                char buf[256];
                snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                         timestamp::now().to_formatted_string().c_str(),
                         buffersToWrite.size() - 2);
                fputs(buf, stderr);
                output.append(buf, static_cast<int>(strlen(buf)));
                buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
            }

            for (const auto &buffer: buffersToWrite) {
                // FIXME: use unbuffered stdio FILE ? or use ::writev ?
                output.append(buffer->data(), buffer->length());
            }

            if (buffersToWrite.size() > 2) {
                // drop non-bzero-ed buffers, avoid trashing
                buffersToWrite.resize(2);
            }

            if (!newBuffer1) {
                assert(!buffersToWrite.empty());
                newBuffer1 = std::move(buffersToWrite.back());
                buffersToWrite.pop_back();
                newBuffer1->reset();
            }

            if (!newBuffer2) {
                assert(!buffersToWrite.empty());
                newBuffer2 = std::move(buffersToWrite.back());
                buffersToWrite.pop_back();
                newBuffer2->reset();
            }

            buffersToWrite.clear();
            output.flush();
        }
        output.flush();
    }

}
