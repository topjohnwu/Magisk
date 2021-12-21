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

// template<class Duration>
//   using sys_time  = time_point<system_clock, Duration>;
// using sys_seconds = sys_time<seconds>;
// using sys_days    = sys_time<days>;

// [Example: 
//   sys_seconds{sys_days{1970y/January/1}}.time_since_epoch() is 0s. 
//   sys_seconds{sys_days{2000y/January/1}}.time_since_epoch() is 946’684’800s, which is 10’957 * 86’400s.
// —end example]


#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    using system_clock = std::chrono::system_clock;
    using year         = std::chrono::year;

    using seconds = std::chrono::seconds;
    using minutes = std::chrono::minutes;
    using days    = std::chrono::days;
    
    using sys_seconds = std::chrono::sys_seconds;
    using sys_minutes = std::chrono::sys_time<minutes>;
    using sys_days    = std::chrono::sys_days;

    constexpr std::chrono::month January = std::chrono::January;

    ASSERT_SAME_TYPE(std::chrono::sys_time<seconds>, sys_seconds);
    ASSERT_SAME_TYPE(std::chrono::sys_time<days>,    sys_days);

//  Test the long form, too
    ASSERT_SAME_TYPE(std::chrono::time_point<system_clock, seconds>, sys_seconds);
    ASSERT_SAME_TYPE(std::chrono::time_point<system_clock, minutes>, sys_minutes);
    ASSERT_SAME_TYPE(std::chrono::time_point<system_clock, days>,    sys_days);
    
//  Test some well known values
    sys_days d0 = sys_days{year{1970}/January/1};
    sys_days d1 = sys_days{year{2000}/January/1};
    ASSERT_SAME_TYPE(decltype(d0.time_since_epoch()), days);
    assert( d0.time_since_epoch().count() == 0);
    assert( d1.time_since_epoch().count() == 10957);

    sys_seconds s0{d0};
    sys_seconds s1{d1};
    ASSERT_SAME_TYPE(decltype(s0.time_since_epoch()), seconds);
    assert( s0.time_since_epoch().count() == 0);
    assert( s1.time_since_epoch().count() == 946684800L);
}
