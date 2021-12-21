//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <chrono>
// class year_month_day;

// constexpr operator sys_days() const noexcept;
//
// Returns: If ok(), returns a sys_days holding a count of days from the
//   sys_days epoch to *this (a negative value if *this represents a date
//   prior to the sys_days epoch). Otherwise, if y_.ok() && m_.ok() is true,
//   returns a sys_days which is offset from sys_days{y_/m_/last} by the
//   number of days d_ is offset from sys_days{y_/m_/last}.day(). Otherwise
//   the value returned is unspecified.
//
// Remarks: A sys_days in the range [days{-12687428}, days{11248737}] which
//   is converted to a year_month_day shall have the same value when
//   converted back to a sys_days.
//
// [Example:
//   static_assert(year_month_day{sys_days{2017y/January/0}}  == 2016y/December/31);
//   static_assert(year_month_day{sys_days{2017y/January/31}} == 2017y/January/31);
//   static_assert(year_month_day{sys_days{2017y/January/32}} == 2017y/February/1);
// —end example]

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

void RunTheExample()
{
    using namespace std::chrono;

    static_assert(year_month_day{sys_days{year{2017}/January/0}}  == year{2016}/December/31);
    static_assert(year_month_day{sys_days{year{2017}/January/31}} == year{2017}/January/31);
    static_assert(year_month_day{sys_days{year{2017}/January/32}} == year{2017}/February/1);  
}

int main()
{
    using year           = std::chrono::year;
    using month          = std::chrono::month;
    using day            = std::chrono::day;
    using sys_days       = std::chrono::sys_days;
    using days           = std::chrono::days;
    using year_month_day = std::chrono::year_month_day;

    ASSERT_NOEXCEPT(sys_days(std::declval<year_month_day>()));
    RunTheExample();

    {
    constexpr year_month_day ymd{year{1970}, month{1}, day{1}};
    constexpr sys_days sd{ymd};

    static_assert( sd.time_since_epoch() == days{0}, "");
    static_assert( year_month_day{sd} == ymd, ""); // and back
    }

    {
    constexpr year_month_day ymd{year{2000}, month{2}, day{2}};
    constexpr sys_days sd{ymd};

    static_assert( sd.time_since_epoch() == days{10957+32}, "");
    static_assert( year_month_day{sd} == ymd, ""); // and back
    }

//  There's one more leap day between 1/1/40 and 1/1/70
//  when compared to 1/1/70 -> 1/1/2000
    {
    constexpr year_month_day ymd{year{1940}, month{1}, day{2}};
    constexpr sys_days sd{ymd};

    static_assert( sd.time_since_epoch() == days{-10957}, "");
    static_assert( year_month_day{sd} == ymd, ""); // and back
    }

    {
    year_month_day ymd{year{1939}, month{11}, day{29}};
    sys_days sd{ymd};

    assert( sd.time_since_epoch() == days{-(10957+34)});
    assert( year_month_day{sd} == ymd); // and back
    }

}
