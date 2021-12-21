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
//   operator+(const year_month_day_last& ymdl, const months& dm) noexcept;
//
//   Returns: (ymdl.year() / ymdl.month() + dm) / last.
//
// constexpr year_month_day_last
//   operator+(const months& dm, const year_month_day_last& ymdl) noexcept;
//
//   Returns: ymdl + dm.
//
//
// constexpr year_month_day_last
//   operator+(const year_month_day_last& ymdl, const years& dy) noexcept;
//
//   Returns: {ymdl.year()+dy, ymdl.month_day_last()}.
//
// constexpr year_month_day_last
//   operator+(const years& dy, const year_month_day_last& ymdl) noexcept;
//
//   Returns: ymdl + dy



#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

constexpr bool testConstexprYears(std::chrono::year_month_day_last ymdl)
{
    std::chrono::years offset{23};
    if (static_cast<int>((ymdl         ).year()) !=  1)           return false;
    if (static_cast<int>((ymdl + offset).year()) != 24)           return false;
    if (                 (ymdl + offset).month() != ymdl.month()) return false;
    if (static_cast<int>((offset + ymdl).year()) != 24)           return false;
    if (                 (offset + ymdl).month() != ymdl.month()) return false;
    return true;
}


constexpr bool testConstexprMonths(std::chrono::year_month_day_last ymdl)
{
    std::chrono::months offset{6};
    if (static_cast<unsigned>((ymdl         ).month()) !=  1)          return false;
    if (                      (ymdl + offset).year()   != ymdl.year()) return false;
    if (static_cast<unsigned>((ymdl + offset).month()) !=  7)          return false;
    if (static_cast<unsigned>((offset + ymdl).month()) !=  7)          return false;
    if (                      (offset + ymdl).year()   != ymdl.year()) return false;
    return true;
}


int main()
{
    using year                = std::chrono::year;
    using month               = std::chrono::month;
    using month_day_last      = std::chrono::month_day_last;
    using year_month_day_last = std::chrono::year_month_day_last;
    using months              = std::chrono::months;
    using years               = std::chrono::years;

    constexpr month January = std::chrono::January;

    {   // year_month_day_last + months
    ASSERT_NOEXCEPT(std::declval<year_month_day_last>() + std::declval<months>());
    ASSERT_NOEXCEPT(std::declval<months>() + std::declval<year_month_day_last>());

    ASSERT_SAME_TYPE(year_month_day_last, decltype(std::declval<year_month_day_last>() + std::declval<months>()));
    ASSERT_SAME_TYPE(year_month_day_last, decltype(std::declval<months>() + std::declval<year_month_day_last>()));

    static_assert(testConstexprMonths(year_month_day_last{year{1}, month_day_last{January}}), "");

    year_month_day_last ym{year{1234}, month_day_last{January}};
    for (int i = 0; i <= 10; ++i)  // TODO test wrap-around
    {
        year_month_day_last ym1 = ym + months{i};
        year_month_day_last ym2 = months{i} + ym;
        assert(static_cast<int>(ym1.year()) == 1234);
        assert(static_cast<int>(ym2.year()) == 1234);
        assert(ym1.month() == month(1 + i));
        assert(ym2.month() == month(1 + i));
        assert(ym1 == ym2);
    }
    }

    {   // year_month_day_last + years
    ASSERT_NOEXCEPT(std::declval<year_month_day_last>() + std::declval<years>());
    ASSERT_NOEXCEPT(std::declval<years>() + std::declval<year_month_day_last>());

    ASSERT_SAME_TYPE(year_month_day_last, decltype(std::declval<year_month_day_last>() + std::declval<years>()));
    ASSERT_SAME_TYPE(year_month_day_last, decltype(std::declval<years>() + std::declval<year_month_day_last>()));

    static_assert(testConstexprYears(year_month_day_last{year{1}, month_day_last{January}}), "");

    year_month_day_last ym{year{1234}, month_day_last{January}};
    for (int i = 0; i <= 10; ++i)
    {
        year_month_day_last ym1 = ym + years{i};
        year_month_day_last ym2 = years{i} + ym;
        assert(static_cast<int>(ym1.year()) == i + 1234);
        assert(static_cast<int>(ym2.year()) == i + 1234);
        assert(ym1.month() == std::chrono::January);
        assert(ym2.month() == std::chrono::January);
        assert(ym1 == ym2);
    }
    }

}
