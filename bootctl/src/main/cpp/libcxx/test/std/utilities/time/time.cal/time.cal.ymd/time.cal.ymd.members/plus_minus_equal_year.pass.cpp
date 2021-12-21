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

// constexpr year_month_day& operator+=(const years& d) noexcept;
// constexpr year_month_day& operator-=(const years& d) noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename D, typename Ds>
constexpr bool testConstexpr(D d1)
{
    if (static_cast<int>((d1          ).year()) !=  1) return false;
    if (static_cast<int>((d1 += Ds{ 1}).year()) !=  2) return false;
    if (static_cast<int>((d1 += Ds{ 2}).year()) !=  4) return false;
    if (static_cast<int>((d1 += Ds{12}).year()) != 16) return false;
    if (static_cast<int>((d1 -= Ds{ 1}).year()) != 15) return false;
    if (static_cast<int>((d1 -= Ds{ 2}).year()) != 13) return false;
    if (static_cast<int>((d1 -= Ds{12}).year()) !=  1) return false;
    return true;
}

int main()
{
    using year           = std::chrono::year;
    using month          = std::chrono::month;
    using day            = std::chrono::day;
    using year_month_day = std::chrono::year_month_day;
    using years          = std::chrono::years;

    ASSERT_NOEXCEPT(std::declval<year_month_day&>() += std::declval<years>());
    ASSERT_NOEXCEPT(std::declval<year_month_day&>() -= std::declval<years>());

    ASSERT_SAME_TYPE(year_month_day&, decltype(std::declval<year_month_day&>() += std::declval<years>()));
    ASSERT_SAME_TYPE(year_month_day&, decltype(std::declval<year_month_day&>() -= std::declval<years>()));

    static_assert(testConstexpr<year_month_day, years>(year_month_day{year{1}, month{1}, day{1}}), "");

    for (int i = 1000; i <= 1010; ++i)
    {
        month m{2};
        day   d{23};
        year_month_day ym(year{i}, m, d);
        assert(static_cast<int>((ym += years{2}).year()) == i + 2);
        assert(ym.month() == m);
        assert(ym.day()   == d);
        assert(static_cast<int>((ym            ).year()) == i + 2);
        assert(ym.month() == m);
        assert(ym.day()   == d);
        assert(static_cast<int>((ym -= years{1}).year()) == i + 1);
        assert(ym.month() == m);
        assert(ym.day()   == d);
        assert(static_cast<int>((ym            ).year()) == i + 1);
        assert(ym.month() == m);
        assert(ym.day()   == d);
    }
}
