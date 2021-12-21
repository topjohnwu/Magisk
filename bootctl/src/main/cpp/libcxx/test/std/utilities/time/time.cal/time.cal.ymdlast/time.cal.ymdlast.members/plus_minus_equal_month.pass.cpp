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

// constexpr year_month_day_last& operator+=(const months& m) noexcept;
// constexpr year_month_day_last& operator-=(const months& m) noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename D, typename Ds>
constexpr bool testConstexpr(D d1)
{
    if (static_cast<unsigned>((d1          ).month()) !=  1) return false;
    if (static_cast<unsigned>((d1 += Ds{ 1}).month()) !=  2) return false;
    if (static_cast<unsigned>((d1 += Ds{ 2}).month()) !=  4) return false;
    if (static_cast<unsigned>((d1 += Ds{12}).month()) !=  4) return false;
    if (static_cast<unsigned>((d1 -= Ds{ 1}).month()) !=  3) return false;
    if (static_cast<unsigned>((d1 -= Ds{ 2}).month()) !=  1) return false;
    if (static_cast<unsigned>((d1 -= Ds{12}).month()) !=  1) return false;
    return true;
}

int main()
{
    using year                = std::chrono::year;
    using month               = std::chrono::month;
    using month_day_last      = std::chrono::month_day_last;
    using year_month_day_last = std::chrono::year_month_day_last;
    using months              = std::chrono::months;

    ASSERT_NOEXCEPT(std::declval<year_month_day_last&>() += std::declval<months>());
    ASSERT_NOEXCEPT(std::declval<year_month_day_last&>() -= std::declval<months>());

    ASSERT_SAME_TYPE(year_month_day_last&, decltype(std::declval<year_month_day_last&>() += std::declval<months>()));
    ASSERT_SAME_TYPE(year_month_day_last&, decltype(std::declval<year_month_day_last&>() -= std::declval<months>()));

    static_assert(testConstexpr<year_month_day_last, months>(year_month_day_last{year{1234}, month_day_last{month{1}}}), "");

    for (unsigned i = 0; i <= 10; ++i)
    {
        year y{1234};
        month_day_last   mdl{month{i}};
        year_month_day_last ym(y, mdl);
        assert(static_cast<unsigned>((ym += months{2}).month()) == i + 2);
        assert(ym.year() == y);
        assert(static_cast<unsigned>((ym             ).month()) == i + 2);
        assert(ym.year() == y);
        assert(static_cast<unsigned>((ym -= months{1}).month()) == i + 1);
        assert(ym.year() == y);
        assert(static_cast<unsigned>((ym             ).month()) == i + 1);
        assert(ym.year() == y);
    }
}
