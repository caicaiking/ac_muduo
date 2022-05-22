//
// Created by acai on 5/22/22.
//

#ifndef AC_MUDUO_WEAK_CALLBACK_H
#define AC_MUDUO_WEAK_CALLBACK_H

#include <memory>
#include <functional>

namespace ac_muduo {
    template<typename CLASS, typename ...ARGS>
    class weak_callback {
    public:
        weak_callback(const std::weak_ptr<CLASS> &ptr, const std::function<void(CLASS *, ARGS...)> &fu) :
                object_(ptr), function_(fu) {}

        void operator()(ARGS &&... args) const {
            auto sptr = this->object_.lock();
            if (sptr) {
                this->function_(sptr.get(), std::forward<ARGS>(args)...);
            }
        }

    private:
        std::weak_ptr<CLASS> object_;
        std::function<void(CLASS *, ARGS...)> function_;
    };

    template<typename CLASS, typename ...ARGS>
    weak_callback<CLASS, ARGS...> make_weak_callback(const std::shared_ptr<CLASS> &object,
                                                     void(CLASS::*function)(ARGS...)
    ) {
        return weak_callback<CLASS, ARGS...>(object, function);
    }

    template<typename CLASS, typename ...ARGS>
    weak_callback<CLASS, ARGS...> make_weak_callback(const std::shared_ptr<CLASS> &object,
                                                     void(CLASS::*function)(ARGS...) const
    ) {
        return weak_callback<CLASS, ARGS...>(object, function);
    }
}
#endif //AC_MUDUO_WEAK_CALLBACK_H
