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

// constexpr operator sys_days() const noexcept;
//  Returns: If ok() == true, returns a sys_days that represents the last weekday() 
//             of year()/month(). Otherwise the returned value is unspecified.

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

#include <iostream>

int main()
{
    using year                    = std::chrono::year;
    using month                   = std::chrono::month;
    using year_month_weekday_last = std::chrono::year_month_weekday_last;
    using sys_days                = std::chrono::sys_days;
    using days                    = std::chrono::days;
    using weekday                 = std::chrono::weekday;
    using weekday_last            = std::chrono::weekday_last;

    ASSERT_NOEXCEPT(                    static_cast<sys_days>(std::declval<const year_month_weekday_last>()));
    ASSERT_SAME_TYPE(sys_days, decltype(static_cast<sys_days>(std::declval<const year_month_weekday_last>())));

    constexpr month   January = std::chrono::January;
    constexpr weekday Tuesday = std::chrono::Tuesday;

    { // Last Tuesday in Jan 1970 was the 27th
    constexpr year_month_weekday_last ymwdl{year{1970}, January, weekday_last{Tuesday}};
    constexpr sys_days sd{ymwdl};
    
    static_assert(sd.time_since_epoch() == days{26}, "");
    }

    { // Last Tuesday in Jan 2000 was the 25th
    constexpr year_month_weekday_last ymwdl{year{2000}, January, weekday_last{Tuesday}};
    constexpr sys_days sd{ymwdl};
    
    static_assert(sd.time_since_epoch() == days{10957+24}, "");
    }

    { // Last Tuesday in Jan 1940 was the 30th
    constexpr year_month_weekday_last ymwdl{year{1940}, January, weekday_last{Tuesday}};
    constexpr sys_days sd{ymwdl};
    
    static_assert(sd.time_since_epoch() == days{-10958+29}, "");
    }

    { // Last Tuesday in Nov 1939 was the 28th
    year_month_weekday_last ymdl{year{1939}, std::chrono::November, weekday_last{Tuesday}};
    sys_days sd{ymdl};

    assert(sd.time_since_epoch() == days{-(10957+35)});
    }
}
