//
// Created by acai on 6/3/22.
//

#ifndef AC_MUDUO_THREAD_LOCAL_SINGLETON_H
#define AC_MUDUO_THREAD_LOCAL_SINGLETON_H

#include "noncopyable.h"
#include <pthread.h>
#include <cassert>

namespace ac_muduo {
    template<class T>
    class thread_local_singleton : public noncopyable {
    public:
        thread_local_singleton() = delete;

        ~thread_local_singleton() = delete;

        static T &instance() {
            if (!t_value_) {
                t_value_ = new T();
                deleter_.set(t_value_);
            }
            return *t_value_;
        }

        static T *pointer() {
            return t_value_;
        }

    private:
        static void destructor(void *obj) {
            assert(obj == t_value_);
            typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
            T_must_be_complete_type dummy;
            (void) dummy;
            delete t_value_;
            t_value_ = nullptr;
        }

    private:
        class deleter {
        public:
            deleter() {
                pthread_key_create(&pkey_, &thread_local_singleton::destructor);
            }

            ~deleter() {
                pthread_key_delete(pkey_);
            }

            void set(T *new_obj) {
                assert(pthread_getspecific(pkey_) == nullptr);
                pthread_setspecific(pkey_, new_obj);
            }

        private:
            pthread_key_t pkey_;
        };

        static __thread T *t_value_;
        static deleter deleter_;
    };

    template<class T>
    __thread T *thread_local_singleton<T>::t_value_ = nullptr;

    template<class T>
    typename thread_local_singleton<T>::deleter  thread_local_singleton<T>::deleter_;
}

#endif //AC_MUDUO_THREAD_LOCAL_SINGLETON_H
