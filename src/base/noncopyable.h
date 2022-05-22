//
// Created by acai on 5/22/22.
//

#ifndef AC_MUDUO_NONCOPYABLE_H
#define AC_MUDUO_NONCOPYABLE_H
namespace ac_muduo {
    class noncopyable {
    public:
        noncopyable(const noncopyable &) = delete;

        void operator=(const noncopyable &) = delete;

    protected:
        noncopyable() = default;

        ~noncopyable() = default;
    };

}
#endif //AC_MUDUO_NONCOPYABLE_H
