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

// constexpr year_month_day operator-(const year_month_day& ymd, const years& dy) noexcept;
//    Returns: ymd + (-dy)


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

#include <iostream>

constexpr bool test_constexpr ()
{
    std::chrono::year_month_day ym0{std::chrono::year{1234}, std::chrono::January, std::chrono::day{12}};
    std::chrono::year_month_day ym1 = ym0 - std::chrono::years{10};
    return
        ym1.year()  == std::chrono::year{1234-10}
     && ym1.month() == std::chrono::January
     && ym1.day()   == std::chrono::day{12}
        ;
}

int main()
{
    using year           = std::chrono::year;
    using month          = std::chrono::month;
    using day            = std::chrono::day;
    using year_month_day = std::chrono::year_month_day;
    using years          = std::chrono::years;

    ASSERT_NOEXCEPT(                          std::declval<year_month_day>() - std::declval<years>());
    ASSERT_SAME_TYPE(year_month_day, decltype(std::declval<year_month_day>() - std::declval<years>()));

    constexpr month January = std::chrono::January;

    static_assert(test_constexpr(), "");

    year_month_day ym{year{1234}, January, day{10}};
    for (int i = 0; i <= 10; ++i)
    {
        year_month_day ym1 = ym - years{i};
        assert(static_cast<int>(ym1.year()) == 1234 - i);
        assert(ym1.month() == January);
        assert(ym1.day() == day{10});
    }
}
