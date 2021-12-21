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

// constexpr bool ok() const noexcept;
//  Returns: y_.ok() && m_.ok() && wdl_.ok().

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year                    = std::chrono::year;
    using month                   = std::chrono::month;
    using weekday                 = std::chrono::weekday;
    using weekday_last            = std::chrono::weekday_last;
    using year_month_weekday_last = std::chrono::year_month_weekday_last;

    constexpr month January   = std::chrono::January;
    constexpr weekday Tuesday = std::chrono::Tuesday;

    ASSERT_NOEXCEPT(                std::declval<const year_month_weekday_last>().ok());
    ASSERT_SAME_TYPE(bool, decltype(std::declval<const year_month_weekday_last>().ok()));

    static_assert(!year_month_weekday_last{year{-32768}, month{}, weekday_last{weekday{}}}.ok(),  ""); // All three bad

    static_assert(!year_month_weekday_last{year{-32768}, January, weekday_last{Tuesday}}.ok(),    ""); // Bad year
    static_assert(!year_month_weekday_last{year{2019},   month{}, weekday_last{Tuesday}}.ok(),    ""); // Bad month
    static_assert(!year_month_weekday_last{year{2019},   January, weekday_last{weekday{7}}}.ok(), ""); // Bad day

    static_assert(!year_month_weekday_last{year{-32768}, month{}, weekday_last{Tuesday}}.ok(),    ""); // Bad year & month
    static_assert(!year_month_weekday_last{year{2019},   month{}, weekday_last{weekday{7}}}.ok(), ""); // Bad month & day
    static_assert(!year_month_weekday_last{year{-32768}, January, weekday_last{weekday{7}}}.ok(), ""); // Bad year & day

    static_assert( year_month_weekday_last{year{2019},   January, weekday_last{Tuesday}}.ok(),    ""); // All OK

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month_weekday_last ym{year{2019}, January, weekday_last{Tuesday}};
        assert((ym.ok() == weekday_last{Tuesday}.ok()));
    }

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month_weekday_last ym{year{2019}, January, weekday_last{weekday{i}}};
        assert((ym.ok() == weekday_last{weekday{i}}.ok()));
    }

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month_weekday_last ym{year{2019}, month{i}, weekday_last{Tuesday}};
        assert((ym.ok() == month{i}.ok()));
    }

    const int ymax = static_cast<int>(year::max());
    for (int i = ymax - 100; i <= ymax + 100; ++i)
    {
        year_month_weekday_last ym{year{i}, January, weekday_last{Tuesday}};
        assert((ym.ok() == year{i}.ok()));
    }
}
