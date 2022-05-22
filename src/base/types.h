//
// Created by acai on 5/22/22.
//

#ifndef AC_MUDUO_TYPES_H
#define AC_MUDUO_TYPES_H

#include <cstdint>
#include <cstring>
#include <string>

#ifndef NDEBUG

#include <cassert>

#endif

namespace ac_muduo {
    using std::string;

    inline void mem_zero(void *p, size_t n) {
        memset(p, 0, n);
    }

    template<typename To, typename From>
    inline To implicit_cat(const From &f) {
        return f;
    }

    template<typename To, typename From>
    inline To down_cast(From *f) {
        if (false) {
            implicit_cat<From *, To>(0);
        }

        return static_cast<To>(f);
    }

}


#endif //AC_MUDUO_TYPES_H
