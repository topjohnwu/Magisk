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

//  constexpr year_month_day(const sys_days& dp) noexcept;
//
//  Effects:  Constructs an object of type year_month_day that corresponds 
//                to the date represented by dp.
//
//  Remarks: For any value ymd of type year_month_day for which ymd.ok() is true,
//                ymd == year_month_day{sys_days{ymd}} is true.
//
//  constexpr chrono::year   year() const noexcept;
//  constexpr chrono::month month() const noexcept;
//  constexpr bool             ok() const noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year           = std::chrono::year;
    using day            = std::chrono::day;
    using sys_days       = std::chrono::sys_days;
    using days           = std::chrono::days;
    using year_month_day = std::chrono::year_month_day;

    ASSERT_NOEXCEPT(year_month_day{std::declval<sys_days>()});

    {
    constexpr sys_days sd{};
    constexpr year_month_day ymd{sd};

    static_assert( ymd.ok(),                            "");
    static_assert( ymd.year()  == year{1970},           "");
    static_assert( ymd.month() == std::chrono::January, "");
    static_assert( ymd.day()   == day{1},               "");
    }

    {
    constexpr sys_days sd{days{10957+32}};
    constexpr year_month_day ymd{sd};

    static_assert( ymd.ok(),                             "");
    static_assert( ymd.year()  == year{2000},            "");
    static_assert( ymd.month() == std::chrono::February, "");
    static_assert( ymd.day()   == day{2},                "");
    }


//  There's one more leap day between 1/1/40 and 1/1/70
//  when compared to 1/1/70 -> 1/1/2000
    {
    constexpr sys_days sd{days{-10957}};
    constexpr year_month_day ymd{sd};

    static_assert( ymd.ok(),                            "");
    static_assert( ymd.year()  == year{1940},           "");
    static_assert( ymd.month() == std::chrono::January, "");
    static_assert( ymd.day()   == day{2},               "");
    }

    {
    sys_days sd{days{-(10957+34)}};
    year_month_day ymd{sd};

    assert( ymd.ok());
    assert( ymd.year()  == year{1939});
    assert( ymd.month() == std::chrono::November);
    assert( ymd.day()   == day{29});
    }
}
