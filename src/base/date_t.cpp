//
// Created by acai on 6/3/22.
//

#include "date_t.h"
#include <ctime> //struct tm
#include <cstdio> //snprintf

namespace ac_muduo {

    namespace detail {
        char requre_32_bit_integer_at_least[sizeof(int) >= sizeof(int32_t) ? 1 : -1];

        int get_julian_day_number(int year, int month, int day) {
            (void) requre_32_bit_integer_at_least;
            int a = (14 - month) / 12;
            int y = year + 4800 - a;
            int m = month + 12 * a - 3;
            return day + (153 * m + 2) / 5 + y * 365 + y / 4 - y / 100 + y / 400 - 32045;
        }

        struct date_t::year_month_day_t get_year_month_day(int julian_day_number) {
            int a = julian_day_number + 32044;
            int b = (4 * a + 3) / 146097;
            int c = a - ((b * 146097) / 4);
            int d = (4 * c + 3) / 1461;
            int e = c - ((1461 * d) / 4);
            int m = (5 * e + 2) / 153;
            date_t::year_month_day_t ymd{};
            ymd.day = e - ((153 * m + 2) / 5) + 1;
            ymd.month = m + 3 - 12 * (m / 10);
            ymd.year = b * 100 + d - 4800 + (m / 10);
            return ymd;
        }
    }

    const int date_t::k_julian_day_of_1970_01_01 = detail::get_julian_day_number(1970, 1, 1);

    date_t::date_t(int year, int month, int day) : julian_day_number_(detail::get_julian_day_number(year, month, day)) {
    }

    date_t::date_t(const tm &t) :
            julian_day_number_(detail::get_julian_day_number(t.tm_year + 1,
                                                             t.tm_mon + 1,
                                                             t.tm_mday
            )) {
    }

    string date_t::to_iso_string() const {
        char buf[32]{};

        year_month_day_t ymd(this->year_month_day());
        snprintf(buf, sizeof buf, "%4d-%02d-%02d", ymd.year, ymd.month, ymd.day);
        return buf;
    }

    date_t::year_month_day_t date_t::year_month_day() const {
        return detail::get_year_month_day(this->julian_day_number_);
    }
}