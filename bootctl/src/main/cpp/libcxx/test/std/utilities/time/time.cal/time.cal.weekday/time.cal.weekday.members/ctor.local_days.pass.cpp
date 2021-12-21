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
// class weekday;

//  constexpr weekday(const local_days& dp) noexcept;
//
//  Effects:  Constructs an object of type weekday by computing what day
//              of the week  corresponds to the local_days dp, and representing
//              that day of the week in wd_ 
//
//  Remarks: For any value ymd of type year_month_day for which ymd.ok() is true,
//                ymd == year_month_day{sys_days{ymd}} is true.
// 
// [Example: 
//  If dp represents 1970-01-01, the constructed weekday represents Thursday by storing 4 in wd_.
// —end example]

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using local_days = std::chrono::local_days;
    using days       = std::chrono::days;
    using weekday    = std::chrono::weekday;

    ASSERT_NOEXCEPT(weekday{std::declval<local_days>()});

    {
    constexpr local_days sd{}; // 1-Jan-1970 was a Thursday
    constexpr weekday wd{sd};

    static_assert( wd.ok(), "");
    static_assert(static_cast<unsigned>(wd) == 4, "");
    }

    {
    constexpr local_days sd{days{10957+32}}; // 2-Feb-2000 was a Wednesday
    constexpr weekday wd{sd};

    static_assert( wd.ok(), "");
    static_assert(static_cast<unsigned>(wd) == 3, "");
    }


    {
    constexpr local_days sd{days{-10957}}; // 2-Jan-1940 was a Tuesday
    constexpr weekday wd{sd};

    static_assert( wd.ok(), "");
    static_assert(static_cast<unsigned>(wd) == 2, "");
    }

    {
    local_days sd{days{-(10957+34)}}; // 29-Nov-1939 was a Wednesday
    weekday wd{sd};

    assert( wd.ok());
    assert(static_cast<unsigned>(wd) == 3);
    }
}
