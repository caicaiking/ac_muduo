//
// Created by acai on 6/4/22.
//

#ifndef AC_MUDUO_FILE_UTILS_H
#define AC_MUDUO_FILE_UTILS_H

#include "string_piece.h"
#include "noncopyable.h"

namespace ac_muduo {
    namespace file_utils {
        //read small file <64 kB
        class read_small_file : noncopyable {
        public:
            read_small_file(string_arg filename);

            ~read_small_file();

            template<class String>
            int read_to_string(int max_size,
                               String *content,
                               int64_t *file_size,
                               int64_t *modify_time,
                               int64_t *create_time
            );


            int read_to_buffer(int *size);

            const char *buffer() const { return this->buf_; }

            static const int k_buffer_size = 64 * 1024;
        private:
            int fd_;
            int err_;
            char buf_[k_buffer_size];
        };

        template<class String>
        int read_file(string_arg filename,
                      int max_size,
                      String *content,
                      int64_t *filesize = nullptr,
                      int64_t *modify_time = nullptr,
                      int64_t *create_time = nullptr
        ) {
            read_small_file file(filename);
            return file.template read_to_string(max_size, content, filesize, modify_time, create_time);
        }

        class append_file : public noncopyable {
        public:
            explicit append_file(string_arg filename);

            ~append_file();

            void append(const char *logline, size_t len);

            void flush();

            off_t written_bytes() const { return this->written_bytes_; }

        private:
            size_t write(const char *logline, size_t len);

            FILE *fp_;
            char buffer_[64 * 1024];
            off_t written_bytes_;
        };
    }
}

#endif //AC_MUDUO_FILE_UTILS_H
