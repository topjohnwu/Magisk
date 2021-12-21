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

// constexpr bool ok() const noexcept;
//  Returns: m_.ok() && y_.ok().

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year           = std::chrono::year;
    using month          = std::chrono::month;
    using day            = std::chrono::day;
    using year_month_day = std::chrono::year_month_day;

    constexpr month January = std::chrono::January;

    ASSERT_NOEXCEPT(                std::declval<const year_month_day>().ok());
    ASSERT_SAME_TYPE(bool, decltype(std::declval<const year_month_day>().ok()));

    static_assert(!year_month_day{year{-32768}, month{}, day{}}.ok(), ""); // All three bad

    static_assert(!year_month_day{year{-32768}, January, day{1}}.ok(), ""); // Bad year
    static_assert(!year_month_day{year{2019},   month{}, day{1}}.ok(), ""); // Bad month
    static_assert(!year_month_day{year{2019},   January, day{} }.ok(), ""); // Bad day

    static_assert(!year_month_day{year{-32768}, month{}, day{1}}.ok(), ""); // Bad year & month
    static_assert(!year_month_day{year{2019},   month{}, day{} }.ok(), ""); // Bad month & day
    static_assert(!year_month_day{year{-32768}, January, day{} }.ok(), ""); // Bad year & day

    static_assert( year_month_day{year{2019},   January, day{1}}.ok(), ""); // All OK

//  Some months have a 31st
    static_assert( year_month_day{year{2020},   month{ 1}, day{31}}.ok(), "");
    static_assert(!year_month_day{year{2020},   month{ 2}, day{31}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 3}, day{31}}.ok(), "");
    static_assert(!year_month_day{year{2020},   month{ 4}, day{31}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 5}, day{31}}.ok(), "");
    static_assert(!year_month_day{year{2020},   month{ 6}, day{31}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 7}, day{31}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 8}, day{31}}.ok(), "");
    static_assert(!year_month_day{year{2020},   month{ 9}, day{31}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{10}, day{31}}.ok(), "");
    static_assert(!year_month_day{year{2020},   month{11}, day{31}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{12}, day{31}}.ok(), "");

//  Everyone except FEB has a 30th
    static_assert( year_month_day{year{2020},   month{ 1}, day{30}}.ok(), "");
    static_assert(!year_month_day{year{2020},   month{ 2}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 3}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 4}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 5}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 6}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 7}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 8}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{ 9}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{10}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{11}, day{30}}.ok(), "");
    static_assert( year_month_day{year{2020},   month{12}, day{30}}.ok(), "");

    static_assert(!year_month_day{year{2019},   std::chrono::February, day{29}}.ok(), ""); // Not a leap year
    static_assert( year_month_day{year{2020},   std::chrono::February, day{29}}.ok(), ""); // Ok; 2020 is a leap year

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month_day ym{year{2019}, January, day{i}};
        assert( ym.ok() == day{i}.ok());
    }

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month_day ym{year{2019}, month{i}, day{12}};
        assert( ym.ok() == month{i}.ok());
    }

    const int ymax = static_cast<int>(year::max());
    for (int i = ymax - 100; i <= ymax + 100; ++i)
    {
        year_month_day ym{year{i}, January, day{12}};
        assert( ym.ok() == year{i}.ok());
    }
}
