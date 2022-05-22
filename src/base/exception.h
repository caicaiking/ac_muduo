//
// Created by acai on 5/22/22.
//

#ifndef AC_MUDUO_EXCEPTION_H
#define AC_MUDUO_EXCEPTION_H

#include "types.h"
#include <exception>

namespace ac_muduo {

    class exception : public std::exception {
    public:
        exception(string what);

        ~exception() noexcept override = default;

        const char *what() const noexcept override {
            return this->message_.c_str();
        }

        const char *stack_trace() const noexcept {
            return this->stack_.c_str();
        }

    private:
        string message_;
        string stack_;
    };

}

#endif //AC_MUDUO_EXCEPTION_H
