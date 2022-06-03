//
// Created by acai on 6/3/22.
//

#ifndef AC_MUDUO_THREAD_LOCAL_H
#define AC_MUDUO_THREAD_LOCAL_H

#include <pthread.h>
#include "noncopyable.h"

namespace ac_muduo {
    template<class T>
    class thread_local_t : public noncopyable {
    public:
        thread_local_t() {
            pthread_key_create(&pkey_, &thread_local_t::destructor);
        }

        ~thread_local_t() {
            pthread_key_delete(pkey_);
        }

        T &value() {
            T *per_thread_value = static_cast<T *>(pthread_getspecific(pkey_));
            if (!per_thread_value) {
                T *new_object = new T();
                pthread_setspecific(pkey_, new_object);
                per_thread_value = new_object;
            }
            return *per_thread_value;
        }

    private:
        static void destructor(void *x) {
            T *obj = static_cast<T *>(x);
            typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
            T_must_be_complete_type dummy;
            (void) dummy;
            delete obj;
        }

    private:
        pthread_key_t pkey_;
    };

}

#endif //AC_MUDUO_THREAD_LOCAL_H
