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

// constexpr operator local_days() const noexcept;
//  Returns: local_days{sys_days{*this}.time_since_epoch()}.

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year                = std::chrono::year;
    using month_day_last      = std::chrono::month_day_last;
    using year_month_day_last = std::chrono::year_month_day_last;
    using local_days          = std::chrono::local_days;
    using days                = std::chrono::days;

    ASSERT_NOEXCEPT(                      static_cast<local_days>(std::declval<const year_month_day_last>()));
    ASSERT_SAME_TYPE(local_days, decltype(static_cast<local_days>(std::declval<const year_month_day_last>())));

    { // Last day in Jan 1970 was the 31st
    constexpr year_month_day_last ymdl{year{1970}, month_day_last{std::chrono::January}};
    constexpr local_days sd{ymdl};
    
    static_assert(sd.time_since_epoch() == days{30}, "");
    }

    {
    constexpr year_month_day_last ymdl{year{2000}, month_day_last{std::chrono::January}};
    constexpr local_days sd{ymdl};

    static_assert(sd.time_since_epoch() == days{10957+30}, "");
    }

    {
    constexpr year_month_day_last ymdl{year{1940}, month_day_last{std::chrono::January}};
    constexpr local_days sd{ymdl};

    static_assert(sd.time_since_epoch() == days{-10957+29}, "");
    }

    {
    year_month_day_last ymdl{year{1939}, month_day_last{std::chrono::November}};
    local_days sd{ymdl};

    assert(sd.time_since_epoch() == days{-(10957+33)});
    }
}
