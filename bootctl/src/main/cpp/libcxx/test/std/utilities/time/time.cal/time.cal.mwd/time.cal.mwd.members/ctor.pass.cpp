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
// class month_weekday;
//   month_weekday represents the nth weekday of a month, of an as yet unspecified year.

//  constexpr month_weekday(const chrono::month& m, const chrono::weekday_indexed& wdi) noexcept;
//    Effects:  Constructs an object of type month_weekday by initializing m_ with m, and wdi_ with wdi.
//
//  constexpr chrono::month                     month() const noexcept;
//  constexpr chrono::weekday_indexed weekday_indexed() const noexcept;
//  constexpr bool                                 ok() const noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using month_weekday   = std::chrono::month_weekday;
    using month           = std::chrono::month;
    using weekday         = std::chrono::weekday;
    using weekday_indexed = std::chrono::weekday_indexed;

    ASSERT_NOEXCEPT(month_weekday{month{1}, weekday_indexed{weekday{}, 1}});

    constexpr month_weekday md0{month{}, weekday_indexed{}};
    static_assert( md0.month()           == month{},           "");
    static_assert( md0.weekday_indexed() == weekday_indexed{}, "");
    static_assert(!md0.ok(),                                   "");

    constexpr month_weekday md1{std::chrono::January, weekday_indexed{std::chrono::Friday, 4}};
    static_assert( md1.month() == std::chrono::January,                              "");
    static_assert( md1.weekday_indexed() == weekday_indexed{std::chrono::Friday, 4}, "");
    static_assert( md1.ok(),                                                         "");
}
