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

// constexpr year_month operator-(const year_month& ym, const years& dy) noexcept;
// Returns: ym + -dy.
//
// constexpr year_month operator-(const year_month& ym, const months& dm) noexcept;
// Returns: ym + -dm.
//
// constexpr months operator-(const year_month& x, const year_month& y) noexcept;
// Returns: x.year() - y.year() + months{static_cast<int>(unsigned{x.month()}) -
//                                       static_cast<int>(unsigned{y.month()})}


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

#include <iostream>

int main()
{
    using year       = std::chrono::year;
    using years      = std::chrono::years;
    using month      = std::chrono::month;
    using months     = std::chrono::months;
    using year_month = std::chrono::year_month;

    {   // year_month - years
    ASSERT_NOEXCEPT(                      std::declval<year_month>() - std::declval<years>());
    ASSERT_SAME_TYPE(year_month, decltype(std::declval<year_month>() - std::declval<years>()));

//  static_assert(testConstexprYears (year_month{year{1}, month{1}}), "");

    year_month ym{year{1234}, std::chrono::January};
    for (int i = 0; i <= 10; ++i)
    {
        year_month ym1 = ym - years{i};
        assert(static_cast<int>(ym1.year()) == 1234 - i);
        assert(ym1.month() == std::chrono::January);
    }
    }

    {   // year_month - months
    ASSERT_NOEXCEPT(                      std::declval<year_month>() - std::declval<months>());
    ASSERT_SAME_TYPE(year_month, decltype(std::declval<year_month>() - std::declval<months>()));

//  static_assert(testConstexprMonths(year_month{year{1}, month{1}}), "");

    year_month ym{year{1234}, std::chrono::November};
    for (int i = 0; i <= 10; ++i)  // TODO test wrap-around
    {
        year_month ym1 = ym - months{i};
        assert(static_cast<int>(ym1.year()) == 1234);
        assert(ym1.month() == month(11 - i));
    }
    }

    {   // year_month - year_month
    ASSERT_NOEXCEPT(                  std::declval<year_month>() - std::declval<year_month>());
    ASSERT_SAME_TYPE(months, decltype(std::declval<year_month>() - std::declval<year_month>()));

//  static_assert(testConstexprMonths(year_month{year{1}, month{1}}), "");

//  Same year
    year y{2345};
    for (int i = 1; i <= 12; ++i)
        for (int j = 1; j <= 12; ++j)
    {
        months diff = year_month{y, month(i)} - year_month{y, month(j)};
        std::cout << "i: " << i << " j: " << j << " -> " << diff.count() << std::endl;
        assert(diff.count() == i - j);
    }

//  TODO: different year

    }
}
