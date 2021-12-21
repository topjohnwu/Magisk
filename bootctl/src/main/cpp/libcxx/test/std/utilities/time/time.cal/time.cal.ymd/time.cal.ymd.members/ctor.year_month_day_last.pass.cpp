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

//  constexpr year_month_day(const year_month_day_last& ymdl) noexcept;
//
//  Effects:  Constructs an object of type year_month_day by initializing
//              y_ with ymdl.year(), m_ with ymdl.month(), and d_ with ymdl.day().
//
//  constexpr chrono::year   year() const noexcept;
//  constexpr chrono::month month() const noexcept;
//  constexpr chrono::day     day() const noexcept;
//  constexpr bool             ok() const noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year                = std::chrono::year;
    using month               = std::chrono::month;
    using day                 = std::chrono::day;
    using month_day_last = std::chrono::month_day_last;
    using year_month_day_last = std::chrono::year_month_day_last;
    using year_month_day      = std::chrono::year_month_day;

    ASSERT_NOEXCEPT(year_month_day{std::declval<const year_month_day_last>()});

    {
    constexpr year_month_day_last ymdl{year{2019}, month_day_last{month{1}}};
    constexpr year_month_day ymd{ymdl};

    static_assert( ymd.year()  == year{2019}, "");
    static_assert( ymd.month() == month{1},   "");
    static_assert( ymd.day()   == day{31},    "");
    static_assert( ymd.ok(),                  "");
    }

    {
    constexpr year_month_day_last ymdl{year{1970}, month_day_last{month{4}}};
    constexpr year_month_day ymd{ymdl};

    static_assert( ymd.year()  == year{1970}, "");
    static_assert( ymd.month() == month{4},   "");
    static_assert( ymd.day()   == day{30},    "");
    static_assert( ymd.ok(),                  "");
    }

    {
    constexpr year_month_day_last ymdl{year{2000}, month_day_last{month{2}}};
    constexpr year_month_day ymd{ymdl};

    static_assert( ymd.year()  == year{2000}, "");
    static_assert( ymd.month() == month{2},   "");
    static_assert( ymd.day()   == day{29},    "");
    static_assert( ymd.ok(),                  "");
    }

    { // Feb 1900 was NOT a leap year.
    year_month_day_last ymdl{year{1900}, month_day_last{month{2}}};
    year_month_day ymd{ymdl};

    assert( ymd.year()  == year{1900});
    assert( ymd.month() == month{2});
    assert( ymd.day()   == day{28});
    assert( ymd.ok());
    }
}
