//
// Created by acai on 6/9/22.
//

#ifndef AC_MUDUO_CHANNEL_T_H
#define AC_MUDUO_CHANNEL_T_H

#include <string>

#include <functional>
#include "base/timestamp.h"
#include "base/noncopyable.h"
#include "base/types.h"
#include "net/event_loop_t.h"
#include <memory>

namespace ac_muduo::net {
    class channel_t : public noncopyable {
    public:
        typedef std::function<void()> event_callback_t;
        typedef std::function<void(timestamp)> read_event_callback_t;


    public:

        channel_t(event_loop_t *loop, int fd);

        ~channel_t();

        void handle_event(timestamp receive_time);

        void set_read_callback(read_event_callback_t cb) {
            this->read_callback_ = std::move(cb);
        }

        void set_write_callback(event_callback_t cb) {
            this->write_callback_ = std::move(cb);
        }

        void set_close_callback(event_callback_t cb) {
            this->close_callback_ = std::move(cb);
        }

        void set_error_callback(event_callback_t cb) {
            this->error_callback_ = std::move(cb);
        }

        void tie(const std::shared_ptr<void> &);

        int fd() const {
            return this->fd_;
        }

        int events() const {
            return this->events_;
        }

        void set_revents(int revents) {
            this->revents_ = revents;
        }

        bool is_none_events() const {
            return this->events_ == k_none_event;
        }

        void enable_reading() {
            this->events_ |= k_read_event;
            this->update();
        }

        void disable_reading() {
            this->events_ &= ~k_read_event;
            this->update();
        }

        void enable_writing() {
            this->events_ |= k_write_event;
            this->update();
        }

        void disable_writing() {
            this->events_ &= ~k_write_event;
            this->update();
        }

        void disable_all() {
            this->events_ = k_none_event;
            this->update();
        }

        bool is_writing() const {
            return this->events_ & k_write_event;
        }

        bool is_reading() const {
            return this->events_ & k_read_event;
        }


        int index() {
            return this->index_;
        }

        void set_index(int index) {
            this->index_ = index;
        }

        std::string revent_to_string();

        std::string event_to_string();

        void do_not_log_hup() {
            this->log_hup_ = false;
        }

        event_loop_t *owner_loop() {
            return this->loop_;
        }

        void remove();

    private:
        static string events_to_string(int fd, int ev);

        void update();

        void hand_event_with_guard(timestamp receive_time);

        static const int k_none_event;
        static const int k_read_event;
        static const int k_write_event;

        event_loop_t *loop_;
        const int fd_;
        int events_;
        int revents_;
        int index_;
        bool log_hup_;

        std::weak_ptr<void> tie_;
        bool tied_;
        bool event_handling_;
        bool add_to_loop_;
        read_event_callback_t read_callback_;
        event_callback_t write_callback_;
        event_callback_t close_callback_;
        event_callback_t error_callback_;
    };
}

#endif //AC_MUDUO_CHANNEL_T_H
