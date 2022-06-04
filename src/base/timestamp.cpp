//
// Created by acai on 6/4/22.
//

#include "timestamp.h"
#include <sys/time.h>
#include <cstdio>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

namespace ac_muduo
{

    static_assert(sizeof(timestamp) == sizeof(int64_t), "timestamp should be the same as int64_t");

    string timestamp::to_string() const {
        char buf[32]{};
        int64_t seconds = this->micro_seconds_since_epoch_ /k_micro_seconds_per_second;
        int64_t microseconds = this->micro_seconds_since_epoch_ % k_micro_seconds_per_second;
        snprintf(buf, sizeof buf, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
        return buf;
    }

    string timestamp::to_formatted_string(bool show_microseconds) const
    {
       char buf[64] {};
       time_t  seconds = static_cast<time_t>(this->micro_seconds_since_epoch_/k_micro_seconds_per_second);
       struct tm tm_time;
        gmtime_r(&seconds, &tm_time);

        if(show_microseconds)
        {
            int micrsconds = static_cast<int>(micro_seconds_since_epoch_ % k_micro_seconds_per_second);
            snprintf(buf, sizeof buf, "%4d%02d%02d %02d:%02d:%02d.%06d",
                     tm_time.tm_year + 1900, tm_time.tm_mon+1, tm_time.tm_mday,
                     tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, micrsconds
                     );


        }
        else
        {
            snprintf(buf, sizeof buf, "%4d%02d%02d %02d:%02d:%02d",
                     tm_time.tm_year + 1900, tm_time.tm_mon+1, tm_time.tm_mday,
                     tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec
            );

        }
        return buf;
    }


    timestamp timestamp::now() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);

        int64_t  seconds = tv.tv_sec;
        return timestamp(seconds * k_micro_seconds_per_second  + tv.tv_usec);
    }


}
