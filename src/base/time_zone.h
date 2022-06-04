//
// Created by acai on 6/4/22.
//

#ifndef AC_MUDUO_TIME_ZONE_H
#define AC_MUDUO_TIME_ZONE_H

#include "muduo/base/copyable.h"
#include <memory>
#include <time.h>

namespace ac_muduo
{

// time_zone for 1970~2030
    class time_zone : public muduo::copyable
    {
    public:
        explicit time_zone(const char* zonefile);
        time_zone(int eastOfUtc, const char* tzname);  // a fixed timezone
        time_zone() = default;  // an invalid timezone

        // default copy ctor/assignment/dtor are Okay.

        bool valid() const
        {
            // 'explicit operator bool() const' in C++11
            return static_cast<bool>(data_);
        }

        struct tm to_local_time(time_t secondsSinceEpoch) const;
        time_t from_local_time(const struct tm&) const;

        // gmtime(3)
        static struct tm to_utc_time(time_t secondsSinceEpoch, bool yday = false);
        // timegm(3)
        static time_t from_utc_time(const struct tm&);
        // year in [1900..2500], month in [1..12], day in [1..31]
        static time_t from_utc_time(int year, int month, int day,
                                    int hour, int minute, int seconds);

        struct Data;

    private:

        std::shared_ptr<Data> data_;
    };

}  // namespace muduo


#endif //AC_MUDUO_TIME_ZONE_H
