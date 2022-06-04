//
// Created by acai on 6/4/22.
//

#ifndef AC_MUDUO_TIMESTAMP_H
#define AC_MUDUO_TIMESTAMP_H

#include "copyable.h"
#include "types.h"
#include <boost/operators.hpp>


namespace ac_muduo {
    class timestamp : public copyable,
                      public boost::equality_comparable<timestamp>,
                      public boost::less_than_comparable<timestamp> {

    public:
        timestamp() : micro_seconds_since_epoch_(0) {}

        explicit timestamp(int64_t micro_seconds_since_epoch_arg) :
                micro_seconds_since_epoch_(micro_seconds_since_epoch_arg) {}

                void swap(timestamp & that)
                {
            std::swap(this->micro_seconds_since_epoch_, that.micro_seconds_since_epoch_);
                }

                string to_string() const;
        string to_formatted_string(bool show_micro_seconds= true) const;

        bool valid() const {return this->micro_seconds_since_epoch_ > 0;}

        int64_t micro_seconds_since_epoch() const
        {
            return this->micro_seconds_since_epoch_;
        }

        time_t seconds_since_epoch() const
        {
            return static_cast<time_t>(this->micro_seconds_since_epoch_ / k_micro_seconds_per_second);
        }

        static timestamp now();
        static timestamp invalid()
        {
            return timestamp();
        }

        static timestamp from_unix_time(time_t t, int microseconds)
        {
            return timestamp(static_cast<int64_t>(t) * k_micro_seconds_per_second + microseconds);
        }

        static timestamp from_unix_time(time_t t)
        {
            return from_unix_time(t, 0);
        }

        static const int k_micro_seconds_per_second = 1000 * 1000;
    private:
        int64_t micro_seconds_since_epoch_;
    };

    inline bool operator<(timestamp lhs, timestamp rhs)
    {
        return lhs.micro_seconds_since_epoch() < rhs.micro_seconds_since_epoch();
    }

    inline bool operator==(timestamp lhs, timestamp rhs)
    {
        return lhs.micro_seconds_since_epoch() == rhs.micro_seconds_since_epoch();
    }

    inline double time_difference(timestamp high, timestamp low)
    {
        int64_t diff = high.micro_seconds_since_epoch() - low.micro_seconds_since_epoch();
        return static_cast<double>(diff)/timestamp::k_micro_seconds_per_second;
    }

    inline timestamp add_time(timestamp t, double  seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * timestamp::k_micro_seconds_per_second);
        return timestamp(t.micro_seconds_since_epoch() + delta);
    }

}

#endif //AC_MUDUO_TIMESTAMP_H
