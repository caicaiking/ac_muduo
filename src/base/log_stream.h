//
// Created by acai on 6/5/22.
//

#ifndef AC_MUDUO_LOG_STREAM_H
#define AC_MUDUO_LOG_STREAM_H

#include "copyable.h"
#include "string_piece.h"
#include "types.h"
#include <cassert>
#include <cstring>
#include "noncopyable.h"

namespace ac_muduo {
    namespace detail {
        const int k_small_buffer = 4000;
        const int k_large_buffer = 4000 * 1000;

        template<int SIZE>
        class fixed_buffer : noncopyable {
        public:
            fixed_buffer() : cur_(data_) {}

            ~fixed_buffer() {}

            void append(const char *buf, size_t len) {
                if (implicit_cat<size_t>(avail()) > len) {
                    memcpy(this->cur_, buf, len);
                    cur_ += len;
                }
            }

            const char *data() const {
                return this->data_;
            }

            int length() const {
                return static_cast<int>(this->cur_ - data_);
            }

            char *current() {
                return this->cur_;
            }

            int avail() const {
                return static_cast<int>(end() - this->cur_);
            }

            void add(size_t len) {
                this->cur_ += len;
            }

            void reset() {
                this->cur_ = this->data_;
            }

            void bzero() {
                mem_zero(this->data_, sizeof data_);
            }

            string to_string() const {
                return string(data_, length());
            }

            string_piece to_string_piece() const {
                return string_piece(this->data_, length());
            }


        private:
            const char *end() const {
                return this->data_ + sizeof data_;
            }

            char data_[SIZE]{};
            char *cur_;
        };

    }
    class log_stream : public noncopyable {
        typedef log_stream self;
    public:
        typedef detail::fixed_buffer<detail::k_small_buffer> buffer_t;

        self &operator<<(bool v) {
            buffer_.append(v ? "1" : "0", 1);
            return *this;
        }

        self &operator<<(short);

        self &operator<<(unsigned short);

        self &operator<<(int);

        self &operator<<(unsigned int);

        self &operator<<(long);

        self &operator<<(unsigned long);

        self &operator<<(long long);

        self &operator<<(unsigned long long);

        self &operator<<(const void *);

        self &operator<<(float v) {
            *this << static_cast<double>(v);
            return *this;
        }

        self &operator<<(double);

        self &operator<<(char v) {
            this->buffer_.append(&v, 1);
            return *this;
        }

        self &operator<<(const char *str) {
            if (str) {
                buffer_.append(str, strlen(str));
            } else {
                buffer_.append("(null)", 6);
            }
            return *this;
        }

        self &operator<<(const string_piece &v) {
            this->buffer_.append(v.data(), v.size());
            return *this;
        }

        self &operator<<(const string &v) {
            this->buffer_.append(v.data(), v.size());
            return *this;
        }

        self &operator<<(const buffer_t &v) {
            *this << v.to_string_piece();
            return *this;
        }

        void append(const char *data, int len) {
            this->buffer_.append(data, len);
        }

        const buffer_t &buffer() const {
            return this->buffer_;
        }

        void reset_buffer() {
            this->buffer_.reset();
        }

    private:
        void static_check();

        template<class T>
        void format_integer(T);

        buffer_t buffer_{};
        static const int k_max_numeric_size = 48;
    };

    class fmt {
    public:
        template<class T>
        fmt(const char *fmt_str, T val);

        const char *data() const {
            return this->buf_;
        }

        int length() const {
            return this->length_;
        }

    private:
        char buf_[32];
        int length_;
    };

    inline log_stream &operator<<(log_stream &s, const fmt &fmt_s) {
        s.append(fmt_s.data(), fmt_s.length());
        return s;
    }

    inline std::ostream &operator<<(std::ostream &in, const string_piece &p) {
        return in << p.as_string();
    }

    string format_SI(int64_t n);

    string format_IEC(int64_t n);
}

#endif //AC_MUDUO_LOG_STREAM_H
