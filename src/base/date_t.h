//
// Created by acai on 6/3/22.
//

#ifndef AC_MUDUO_DATE_T_H
#define AC_MUDUO_DATE_T_H

#include "types.h"
#include "copyable.h"

struct tm;

namespace ac_muduo {

    class date_t : ac_muduo::copyable {
    public:
        struct year_month_day_t {
            int year;
            int month;
            int day;
        };

        static const int k_days_per_week = 7;
        static const int k_julian_day_of_1970_01_01;

        date_t() : julian_day_number_(0) {}

        date_t(int year, int month, int day);

        explicit date_t(int julian_day_num) : julian_day_number_(julian_day_num) {}

        explicit date_t(const struct tm &);

        void swap(date_t &that) {
            std::swap(this->julian_day_number_, that.julian_day_number_);
        }

        bool valid() const {
            return this->julian_day_number_ > 0;
        }

        string to_iso_string() const;

        struct year_month_day_t year_month_day() const;

        int year() const {
            return year_month_day().year;
        }

        int month() const {
            return year_month_day().month;
        }

        int day() const {
            return year_month_day().day;
        }

        int week_day() const {
            return (this->julian_day_number_ + 1) % k_days_per_week;
        }

        int julian_day_number() const {
            return this->julian_day_number_;
        }

    private:
        int julian_day_number_;
    };


    inline bool operator<(date_t x, date_t y) {
        return x.julian_day_number() < y.julian_day_number();
    }

    inline bool operator==(date_t x, date_t y) {
        return x.julian_day_number() == y.julian_day_number();
    }
}

#endif //AC_MUDUO_DATE_T_H
