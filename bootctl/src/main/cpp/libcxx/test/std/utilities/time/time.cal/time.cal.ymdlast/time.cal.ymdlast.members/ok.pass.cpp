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

// constexpr bool ok() const noexcept;
//  Returns: m_.ok() && y_.ok().

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year                = std::chrono::year;
    using month               = std::chrono::month;
    using month_day_last      = std::chrono::month_day_last;
    using year_month_day_last = std::chrono::year_month_day_last;

    constexpr month January = std::chrono::January;

    ASSERT_NOEXCEPT(                std::declval<const year_month_day_last>().ok());
    ASSERT_SAME_TYPE(bool, decltype(std::declval<const year_month_day_last>().ok()));

    static_assert(!year_month_day_last{year{-32768}, month_day_last{month{}}}.ok(), ""); // both bad
    static_assert(!year_month_day_last{year{-32768}, month_day_last{January}}.ok(), ""); // Bad year
    static_assert(!year_month_day_last{year{2019},   month_day_last{month{}}}.ok(), ""); // Bad month
    static_assert( year_month_day_last{year{2019},   month_day_last{January}}.ok(), ""); // All OK

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month_day_last ym{year{2019}, month_day_last{month{i}}};
        assert( ym.ok() == month{i}.ok());
    }

    const int ymax = static_cast<int>(year::max());
    for (int i = ymax - 100; i <= ymax + 100; ++i)
    {
        year_month_day_last ym{year{i}, month_day_last{January}};
        assert( ym.ok() == year{i}.ok());
    }
}
