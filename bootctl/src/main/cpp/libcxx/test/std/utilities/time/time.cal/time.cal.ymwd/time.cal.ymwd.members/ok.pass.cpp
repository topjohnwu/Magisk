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
// class year_month_weekday;

// constexpr bool ok() const noexcept;
//  Returns: m_.ok() && y_.ok().

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year               = std::chrono::year;
    using month              = std::chrono::month;
    using weekday            = std::chrono::weekday;
    using weekday_indexed    = std::chrono::weekday_indexed;
    using year_month_weekday = std::chrono::year_month_weekday;

    constexpr month January = std::chrono::January;
    constexpr weekday Tuesday = std::chrono::Tuesday;

    ASSERT_NOEXCEPT(                std::declval<const year_month_weekday>().ok());
    ASSERT_SAME_TYPE(bool, decltype(std::declval<const year_month_weekday>().ok()));

    static_assert(!year_month_weekday{}.ok(), "");

    static_assert(!year_month_weekday{year{-32768}, month{}, weekday_indexed{}}.ok(),           ""); // All three bad

    static_assert(!year_month_weekday{year{-32768}, January, weekday_indexed{Tuesday, 1}}.ok(), ""); // Bad year
    static_assert(!year_month_weekday{year{2019},   month{}, weekday_indexed{Tuesday, 1}}.ok(), ""); // Bad month
    static_assert(!year_month_weekday{year{2019},   January, weekday_indexed{} }.ok(),          ""); // Bad day

    static_assert(!year_month_weekday{year{-32768}, month{}, weekday_indexed{Tuesday, 1}}.ok(), ""); // Bad year & month
    static_assert(!year_month_weekday{year{2019},   month{}, weekday_indexed{} }.ok(),          ""); // Bad month & day
    static_assert(!year_month_weekday{year{-32768}, January, weekday_indexed{} }.ok(),          ""); // Bad year & day

    static_assert( year_month_weekday{year{2019},   January, weekday_indexed{Tuesday, 1}}.ok(), ""); // All OK

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month_weekday ym{year{2019}, January, weekday_indexed{Tuesday, i}};
        assert((ym.ok() == weekday_indexed{Tuesday, i}.ok()));
    }

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month_weekday ym{year{2019}, January, weekday_indexed{weekday{i}, 1}};
        assert((ym.ok() == weekday_indexed{weekday{i}, 1}.ok()));
    }

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month_weekday ym{year{2019}, month{i}, weekday_indexed{Tuesday, 1}};
        assert((ym.ok() == month{i}.ok()));
    }

    const int ymax = static_cast<int>(year::max());
    for (int i = ymax - 100; i <= ymax + 100; ++i)
    {
        year_month_weekday ym{year{i}, January, weekday_indexed{Tuesday, 1}};
        assert((ym.ok() == year{i}.ok()));
    }
}
