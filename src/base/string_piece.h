//
// Created by acai on 6/3/22.
//

#ifndef AC_MUDUO_STRING_PIECE_H
#define AC_MUDUO_STRING_PIECE_H

#include "types.h"
#include <iosfwd>

namespace ac_muduo {
    class string_arg {
    public:
        string_arg(const char *str) : str_(str) {}

        string_arg(const string &str) : str_(str.data()) {}

        const char *c_str() const { return this->str_; }

        const char *data() const { return this->str_; }

    private:
        const char *str_;
    };

    class string_piece {
    private:
        const char *ptr_;
        int length_;

    public:
        string_piece() : ptr_(nullptr), length_(0) {}

        string_piece(const char *str) : ptr_(str), length_(static_cast<int> (strlen(str))) {}

        string_piece(const unsigned char *str) :
                ptr_(reinterpret_cast<const char *>(str)), length_(static_cast<int>(strlen(ptr_))) {}

        string_piece(const string &str) : ptr_(str.data()), length_(static_cast<int> (str.length())) {}

        string_piece(const char *offset, int len) : ptr_(offset), length_(len) {}

        const char *data() const { return this->ptr_; }

        int size() const { return this->length_; }

        bool empty() const { return this->length_ == 0; }

        const char *begin() const { return this->ptr_; }

        const char *end() const { return this->ptr_ + this->length_; }

        void clear() {
            this->ptr_ = nullptr;
            this->length_ = 0;
        }

        void set(const char *buffer, int len) {
            this->ptr_ = buffer;
            this->length_ = len;
        }

        void set(const char *str) {
            this->ptr_ = str;
            this->length_ = static_cast<int>(strlen(str));
        }

        void set(const void *buffer, int len) {
            this->ptr_ = static_cast<const char *>(buffer);
            this->length_ = len;
        }

        char operator[](int i) const {
            return this->ptr_[i];
        }

        void remove_prefix(int n) {
            this->ptr_ += n;
            this->length_ -= n;
        }

        void remove_suffix(int n) {
            this->length_ -= n;
        }

        bool operator==(const string_piece &x) const {
            return ((this->length_ == x.length_)
                    && (memcmp(this->ptr_, x.ptr_, this->length_) == 0));
        }

        bool operator!=(const string_piece &x) const {
            return !(*this == x);
        }

#define STRINGPIECE_BINARY_PREDICATE(cmp, auxcmp)                             \
  bool operator cmp (const string_piece& x) const {                           \
    int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_); \
    return ((r auxcmp 0) || ((r == 0) && (length_ cmp x.length_)));          \
  }

        STRINGPIECE_BINARY_PREDICATE(<, <);

        STRINGPIECE_BINARY_PREDICATE(<=, <);

        STRINGPIECE_BINARY_PREDICATE(>=, >);

        STRINGPIECE_BINARY_PREDICATE(>, >);
#undef STRINGPIECE_BINARY_PREDICATE


        int compare(const string_piece &x) const {

            int r = memcmp(this->ptr_, x.ptr_, this->length_ < x.length_ ? this->length_ : x.length_);
            if (r == 0) {
                if (this->length_ < x.length_) r = -1;
                else if (this->length_ > x.length_) r = +1;
            }

            return r;
        }

        string as_string() const {
            return string(this->ptr_, this->length_);
        }

        void copy_to_string(string *target) const {
            target->assign(this->ptr_, this->length_);
        }

        bool start_with(const string_piece &x) const {
            return ((this->length_ >= x.length_) && (memcmp(this->ptr_, x.ptr_, x.length_) == 0));
        }


    };

#ifdef HAVE_TYPE_TRAITS
    // This makes vector<StringPiece> really fast for some STL implementations
template<> struct __type_traits<ac_muduo::string_piece> {
typedef __true_type    has_trivial_default_constructor;
typedef __true_type    has_trivial_copy_constructor;
typedef __true_type    has_trivial_assignment_operator;
typedef __true_type    has_trivial_destructor;
typedef __true_type    is_POD_type;
};
#endif

    std::ostream &operator<<(std::ostream &o, const ac_muduo::string_piece &piece);

}


#endif //AC_MUDUO_STRING_PIECE_H
