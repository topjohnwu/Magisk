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
// class year_month_weekday_last;

// constexpr year_month_weekday_last operator-(const year_month_weekday_last& ymwdl, const months& dm) noexcept;
//   Returns: ymwdl + (-dm).
//
// constexpr year_month_weekday_last operator-(const year_month_weekday_last& ymwdl, const years& dy) noexcept;
//   Returns: ymwdl + (-dy).

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

#include <iostream>

constexpr bool testConstexprYears(std::chrono::year_month_weekday_last ym)
{
    std::chrono::years offset{14};
    if (static_cast<int>((ym         ).year()) != 66) return false;
    if (static_cast<int>((ym - offset).year()) != 52) return false;
    return true;
}

constexpr bool testConstexprMonths(std::chrono::year_month_weekday_last ym)
{
    std::chrono::months offset{6};
    if (static_cast<unsigned>((ym         ).month()) != 10) return false;
    if (static_cast<unsigned>((ym - offset).month()) !=  4) return false;
    return true;
}

int main()
{
    using year                    = std::chrono::year;
    using month                   = std::chrono::month;
    using weekday                 = std::chrono::weekday;
    using weekday_last            = std::chrono::weekday_last;
    using year_month_weekday_last = std::chrono::year_month_weekday_last;
    using years                   = std::chrono::years;
    using months                  = std::chrono::months;

    constexpr month October = std::chrono::October;
    constexpr weekday Tuesday = std::chrono::Tuesday;

    { // year_month_weekday_last - years

    ASSERT_NOEXCEPT(                                   std::declval<year_month_weekday_last>() - std::declval<years>());
    ASSERT_SAME_TYPE(year_month_weekday_last, decltype(std::declval<year_month_weekday_last>() - std::declval<years>()));

    static_assert(testConstexprYears(year_month_weekday_last{year{66}, October, weekday_last{Tuesday}}), "");

    year_month_weekday_last ym{year{1234}, October, weekday_last{Tuesday}};
    for (int i = 0; i <= 10; ++i)
    {
        year_month_weekday_last ym1 = ym - years{i};
        assert(ym1.year()         == year{1234 - i});
        assert(ym1.month()        == October);
        assert(ym1.weekday()      == Tuesday);
        assert(ym1.weekday_last() == weekday_last{Tuesday});
    }
    }

    { // year_month_weekday_last - months

    ASSERT_NOEXCEPT(                                   std::declval<year_month_weekday_last>() - std::declval<months>());
    ASSERT_SAME_TYPE(year_month_weekday_last, decltype(std::declval<year_month_weekday_last>() - std::declval<months>()));

    static_assert(testConstexprMonths(year_month_weekday_last{year{66}, October, weekday_last{Tuesday}}), "");

    year_month_weekday_last ym{year{1234}, October, weekday_last{Tuesday}};
    for (unsigned i = 0; i < 10; ++i)
    {
        year_month_weekday_last ym1 = ym - months{i};
        assert(ym1.year()         == year{1234});
        assert(ym1.month()        == month{10 - i});
        assert(ym1.weekday()      == Tuesday);
        assert(ym1.weekday_last() == weekday_last{Tuesday});
    }
    }

}
