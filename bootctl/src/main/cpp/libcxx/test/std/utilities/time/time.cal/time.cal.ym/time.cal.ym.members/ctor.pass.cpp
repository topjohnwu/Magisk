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
// class year_month;

//            year_month() = default;
//  constexpr year_month(const chrono::year& y, const chrono::month& m) noexcept;
//
//  Effects:  Constructs an object of type year_month by initializing y_ with y, and m_ with m.
//
//  constexpr chrono::year   year() const noexcept;
//  constexpr chrono::month month() const noexcept;
//  constexpr bool             ok() const noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year       = std::chrono::year;
    using month      = std::chrono::month;
    using year_month = std::chrono::year_month;

    ASSERT_NOEXCEPT(year_month{});
    ASSERT_NOEXCEPT(year_month{year{1}, month{1}});

    constexpr year_month ym0{};
    static_assert( ym0.year()  == year{},  "");
    static_assert( ym0.month() == month{}, "");
    static_assert(!ym0.ok(),               "");

    constexpr year_month ym1{year{2018}, std::chrono::January};
    static_assert( ym1.year()  == year{2018},           "");
    static_assert( ym1.month() == std::chrono::January, "");
    static_assert( ym1.ok(),                            "");

    constexpr year_month ym2{year{2018}, month{}};
    static_assert( ym2.year()  == year{2018}, "");
    static_assert( ym2.month() == month{},    "");
    static_assert(!ym2.ok(),                  "");
}
