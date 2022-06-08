//
// Created by acai on 6/8/22.
//

#ifndef AC_MUDUO_GZIP_FILE_H
#define AC_MUDUO_GZIP_FILE_H

#include "string_piece.h"
#include "noncopyable.h"
#include <zlib.h>

namespace ac_muduo {
    class gzip_file : public noncopyable {
        gzip_file(gzip_file &&rhs) noexcept:
                file_(rhs.file_) {
            rhs.file_ = nullptr;
        }

        ~ gzip_file() {
            if (this->file_) {
                ::gzclose(this->file_);
            }
        }

        gzip_file &operator=(gzip_file &&rhs) noexcept {
            this->swap(rhs);
            return *this;
        }

        bool valid() const {
            return this->file_ != nullptr;
        }

        void swap(gzip_file &rhs) {
            std::swap(this->file_, rhs.file_);
        }

#if ZLIB_VERNUM >= 0x1240

        bool set_buffer(int size) {
            return ::gzbuffer(this->file_, size) == 0;
        }

#endif

        int read(void *buf, int len) {
            return ::gzread(this->file_, buf, len);
        }

        int write(string_piece buf) {
            return ::gzwrite(this->file_, buf.data(), buf.size());
        }

        off_t tell() const {
            return ::gztell(this->file_);
        }

#if ZLIB_VERNUM >= 0x1240

        off_t offset() const { return ::gzoffset(file_); }

#endif

        static gzip_file open_for_read(string_arg filename) {
            return gzip_file(::gzopen(filename.c_str(), "rbe"));
        }

        static gzip_file open_for_append(string_arg filename) {
            return gzip_file(::gzopen(filename.c_str(), "abe"));
        }

        static gzip_file open_for_write_exclusive(string_arg filename) {
            return gzip_file(::gzopen(filename.c_str(), "wbxe"));
        }

        static gzip_file open_for_write_truncate(string_arg filename) {
            return gzip_file(::gzopen(filename.c_str(), "wbe"));
        }


    private:
        explicit gzip_file(gzFile file) : file_(file) {}

        gzFile file_;
    };

}

#endif //AC_MUDUO_GZIP_FILE_H
