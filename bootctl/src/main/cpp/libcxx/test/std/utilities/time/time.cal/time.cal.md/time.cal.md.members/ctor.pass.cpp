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
// class month_day;

//                     month_day() = default;
//  constexpr month_day(const chrono::month& m, const chrono::day& d) noexcept;
//
//  Effects:  Constructs an object of type month_day by initializing m_ with m, and d_ with d.
//
//  constexpr chrono::month month() const noexcept;
//  constexpr chrono::day     day() const noexcept;
//  constexpr bool             ok() const noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using day       = std::chrono::day;
    using month     = std::chrono::month;
    using month_day = std::chrono::month_day;

    ASSERT_NOEXCEPT(month_day{});
    ASSERT_NOEXCEPT(month_day{month{1}, day{1}});

    constexpr month_day md0{};
    static_assert( md0.month() == month{}, "");
    static_assert( md0.day()   == day{},   "");
    static_assert(!md0.ok(),               "");

    constexpr month_day md1{std::chrono::January, day{4}};
    static_assert( md1.month() == std::chrono::January, "");
    static_assert( md1.day()   == day{4},               "");
    static_assert( md1.ok(),                            "");
}
