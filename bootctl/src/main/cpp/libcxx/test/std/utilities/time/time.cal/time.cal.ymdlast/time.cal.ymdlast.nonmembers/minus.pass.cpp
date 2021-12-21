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

// constexpr year_month_day_last
//   operator-(const year_month_day_last& ymdl, const months& dm) noexcept;
//
//   Returns: ymdl + (-dm).
//
// constexpr year_month_day_last
//   operator-(const year_month_day_last& ymdl, const years& dy) noexcept;
//
//   Returns: ymdl + (-dy).


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

#include <iostream>

constexpr bool testConstexprYears (std::chrono::year_month_day_last ymdl)
{
    std::chrono::year_month_day_last ym1 = ymdl - std::chrono::years{10};
    return
        ym1.year()  == std::chrono::year{static_cast<int>(ymdl.year()) - 10}
     && ym1.month() == ymdl.month()
        ;
}

constexpr bool testConstexprMonths (std::chrono::year_month_day_last ymdl)
{
    std::chrono::year_month_day_last ym1 = ymdl - std::chrono::months{6};
    return
        ym1.year()  == ymdl.year()
     && ym1.month() == std::chrono::month{static_cast<unsigned>(ymdl.month()) - 6}
        ;
}

int main()
{
    using year                = std::chrono::year;
    using month               = std::chrono::month;
    using month_day_last      = std::chrono::month_day_last;
    using year_month_day_last = std::chrono::year_month_day_last;
    using months              = std::chrono::months;
    using years               = std::chrono::years;

    constexpr month December = std::chrono::December;

    { // year_month_day_last - years
    ASSERT_NOEXCEPT(                               std::declval<year_month_day_last>() - std::declval<years>());
    ASSERT_SAME_TYPE(year_month_day_last, decltype(std::declval<year_month_day_last>() - std::declval<years>()));

    static_assert(testConstexprYears(year_month_day_last{year{1234}, month_day_last{December}}), "");
    year_month_day_last ym{year{1234}, month_day_last{December}};
    for (int i = 0; i <= 10; ++i)
    {
        year_month_day_last ym1 = ym - years{i};
        assert(static_cast<int>(ym1.year()) == 1234 - i);
        assert(ym1.month() == December);
    }
    }

    { // year_month_day_last - months
    ASSERT_NOEXCEPT(                               std::declval<year_month_day_last>() - std::declval<months>());
    ASSERT_SAME_TYPE(year_month_day_last, decltype(std::declval<year_month_day_last>() - std::declval<months>()));

    static_assert(testConstexprMonths(year_month_day_last{year{1234}, month_day_last{December}}), "");
//  TODO test wrapping
    year_month_day_last ym{year{1234}, month_day_last{December}};
    for (unsigned i = 0; i <= 10; ++i)
    {
        year_month_day_last ym1 = ym - months{i};
        assert(static_cast<int>(ym1.year()) == 1234);
        assert(static_cast<unsigned>(ym1.month()) == 12U-i);
    }
    }

}
