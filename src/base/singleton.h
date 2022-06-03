//
// Created by acai on 6/3/22.
//

#ifndef AC_MUDUO_SINGLETON_H
#define AC_MUDUO_SINGLETON_H

#include "noncopyable.h"
#include <future>

namespace ac_muduo {
    namespace detail {
        template<class T>
        struct has_no_destory {
            template<class C>
            static char test(decltype(&C::no_destory));

            template<class C>
            static int test(...);

            const static bool value = sizeof(test<T>(0)) == 1;
        };

        template<class T>
        class singleton_t : public noncopyable {
        public:
            singleton_t() = delete;

            ~singleton_t() = delete;

            static T &instance() {
                std::call_once(once_, init);
                assert(value_ != nullptr);
                return *value_;
            }

        private:
            static void init() {
                value_ = new T();
                if (!detail::has_no_destory<T>::value) {
                    ::atexit(destroy);
                }
            }

            static void destroy() {
                typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
                T_must_be_complete_type dummy;
                (void) dummy;

                delete value_;
                value_ == nullptr;
            }

        private:
            static std::once_flag once_;
            static T *value_;
        };

        template<class T>
        std::once_flag singleton_t<T>::once_;

        template<class T>
        T *singleton_t<T>::value_ = nullptr;
    }

}
#endif //AC_MUDUO_SINGLETON_H
