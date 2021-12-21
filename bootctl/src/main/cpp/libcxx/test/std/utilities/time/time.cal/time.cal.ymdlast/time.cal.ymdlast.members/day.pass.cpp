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
// class year_month_day_last;

// constexpr chrono::day day() const noexcept;
//  Returns: wd_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year                = std::chrono::year;
    using month               = std::chrono::month;
    using day                 = std::chrono::day;
    using month_day_last      = std::chrono::month_day_last;
    using year_month_day_last = std::chrono::year_month_day_last;

    ASSERT_NOEXCEPT(               std::declval<const year_month_day_last>().day());
    ASSERT_SAME_TYPE(day, decltype(std::declval<const year_month_day_last>().day()));

//  Some months have a 31st
    static_assert( year_month_day_last{year{2020}, month_day_last{month{ 1}}}.day() == day{31}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{ 2}}}.day() == day{29}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{ 3}}}.day() == day{31}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{ 4}}}.day() == day{30}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{ 5}}}.day() == day{31}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{ 6}}}.day() == day{30}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{ 7}}}.day() == day{31}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{ 8}}}.day() == day{31}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{ 9}}}.day() == day{30}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{10}}}.day() == day{31}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{11}}}.day() == day{30}, "");
    static_assert( year_month_day_last{year{2020}, month_day_last{month{12}}}.day() == day{31}, "");

    assert((year_month_day_last{year{2019}, month_day_last{month{ 2}}}.day() == day{28}));
    assert((year_month_day_last{year{2020}, month_day_last{month{ 2}}}.day() == day{29}));
    assert((year_month_day_last{year{2021}, month_day_last{month{ 2}}}.day() == day{28}));
}
