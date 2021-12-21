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

//  year_month_day() = default;
//  constexpr year_month_day(const chrono::year& y, const chrono::month& m,
//                                   const chrono::day& d) noexcept;
//
//  Effects:  Constructs an object of type year_month_day by initializing
//                y_ with y, m_ with m, and d_ with d.
//
//  constexpr chrono::year   year() const noexcept;
//  constexpr chrono::month month() const noexcept;
//  constexpr chrono::day     day() const noexcept;
//  constexpr bool             ok() const noexcept;

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

    ASSERT_NOEXCEPT(year_month_day{});
    ASSERT_NOEXCEPT(year_month_day{year{1}, month{1}, day{1}});

    constexpr month January = std::chrono::January;

    constexpr year_month_day ym0{};
    static_assert( ym0.year()  == year{},  "");
    static_assert( ym0.month() == month{}, "");
    static_assert( ym0.day()   == day{},   "");
    static_assert(!ym0.ok(),               "");

    constexpr year_month_day ym1{year{2019}, January, day{12}};
    static_assert( ym1.year()  == year{2019}, "");
    static_assert( ym1.month() == January,    "");
    static_assert( ym1.day()   == day{12},    "");
    static_assert( ym1.ok(),                  "");

}
