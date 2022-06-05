//
// Created by acai on 6/4/22.
//

#include "file_utils.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cassert>

ac_muduo::file_utils::read_small_file::read_small_file(ac_muduo::string_arg filename)
        : fd_(::open(filename.c_str(), O_RDONLY | O_CLOEXEC)), err_(0) {
    buf_[0] = '\0';
    if (this->fd_ < 0) {
        this->err_ = errno;
    }
}

ac_muduo::file_utils::read_small_file::~read_small_file() {
    if (this->fd_ >= 0) {
        ::close(this->fd_);
    }
}

int ac_muduo::file_utils::read_small_file::read_to_buffer(int *size) {
    int err = this->err_;
    if (this->fd_ >= 0) {
        ssize_t n = ::pread(this->fd_, this->buf_, sizeof(this->buf_) - 1, 0);
        if (n >= 0) {
            if (size) {
                *size = static_cast<int>(n);
            }
            this->buf_[n] = '\0';
        } else {
            err = errno;
        }
    }
    return err;
}

ac_muduo::file_utils::append_file::append_file(ac_muduo::string_arg filename) :
        fp_(::fopen(filename.c_str(), "ae")),//'e' for O_CLOEXEC
        written_bytes_(0) {
    assert(fp_);
    ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

ac_muduo::file_utils::append_file::~append_file() {
    ::fclose(fp_);
}

void ac_muduo::file_utils::append_file::append(const char *logline, size_t len) {
    size_t written = 0;
    while (written != len) {
        size_t remain = len - written;
        size_t n = write(logline + written, remain);
        if (n != remain) {
            int err = ferror(fp_);
            if (err) {
                fprintf(stderr, "append_file::append() failed %s", strerror(err));
                break;
            }
        }
        written += n;
    }

    this->written_bytes_ += written;
}

void ac_muduo::file_utils::append_file::flush() {
    ::fflush(this->fp_);
}

size_t ac_muduo::file_utils::append_file::write(const char *logline, size_t len) {
    //not thread safe, fwrite is thread safe.
    return ::fwrite_unlocked(logline, 1, len, fp_);

}

template<class String>
int ac_muduo::file_utils::read_small_file::read_to_string(int max_size,
                                                          String *content,
                                                          int64_t *file_size,
                                                          int64_t *modify_time,
                                                          int64_t *create_time
) {
    static_assert(sizeof(off_t) == 8, "_FILE_OFFSET_BITS = 64");
    assert(content != nullptr);
    int err = this->err_;

    if (fd_ >= 0) {
        if (file_size) {
            struct stat statbuf;
            if (::fstat(this->fd_, &statbuf) == 0) {
                if (S_ISREG(statbuf.st_mode)) {
                    *file_size = statbuf.st_size;
                    content->reserve(static_cast<int>(std::min(static_cast<int64_t>(max_size), *file_size)));
                } else if (S_ISDIR(statbuf.st_mode)) {
                    err = EISDIR;
                }

                if (modify_time) {
                    *modify_time = statbuf.st_mtime;
                }

                if (create_time) {
                    *create_time = statbuf.st_ctime;
                }
            } else {
                err = errno;
            }
        }

        while (content->size() < ac_muduo::implicit_cat<size_t>(max_size)) {
            size_t to_read = std::min(ac_muduo::implicit_cat<size_t>(max_size) - content->size(), sizeof(this->buf_));

            ssize_t n = ::read(this->fd_, buf_, to_read);
            if (n > 0) {
                content->append(buf_, n);
            } else {
                if (n < 0) {
                    err = errno;
                }
                break;
            }
        }

    }
    return err;
}

template
int ac_muduo::file_utils::read_small_file::read_to_string(int max_size,
                                                          string *content,
                                                          int64_t *file_size,
                                                          int64_t *modify_time,
                                                          int64_t *create_time);

template
int ac_muduo::file_utils::read_file(string_arg filename,
                                    int max_size,
                                    string *content,
                                    int64_t *filesize = nullptr,
                                    int64_t *modify_time = nullptr,
                                    int64_t *create_time = nullptr);