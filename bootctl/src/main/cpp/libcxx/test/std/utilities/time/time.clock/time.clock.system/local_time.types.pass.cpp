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

// struct local_t {};
// template<class Duration>
//   using local_time  = time_point<system_clock, Duration>;
// using local_seconds = sys_time<seconds>;
// using local_days    = sys_time<days>;

// [Example: 
//   sys_seconds{sys_days{1970y/January/1}}.time_since_epoch() is 0s. 
//   sys_seconds{sys_days{2000y/January/1}}.time_since_epoch() is 946’684’800s, which is 10’957 * 86’400s.
// —end example]


#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    using local_t = std::chrono::local_t;
    using year    = std::chrono::year;

    using seconds = std::chrono::seconds;
    using minutes = std::chrono::minutes;
    using days    = std::chrono::days;
    
    using local_seconds = std::chrono::local_seconds;
    using local_minutes = std::chrono::local_time<minutes>;
    using local_days    = std::chrono::local_days;

    constexpr std::chrono::month January = std::chrono::January;

    ASSERT_SAME_TYPE(std::chrono::local_time<seconds>, local_seconds);
    ASSERT_SAME_TYPE(std::chrono::local_time<days>,    local_days);

//  Test the long form, too
    ASSERT_SAME_TYPE(std::chrono::time_point<local_t, seconds>, local_seconds);
    ASSERT_SAME_TYPE(std::chrono::time_point<local_t, minutes>, local_minutes);
    ASSERT_SAME_TYPE(std::chrono::time_point<local_t, days>,    local_days);
    
//  Test some well known values
    local_days d0 = local_days{year{1970}/January/1};
    local_days d1 = local_days{year{2000}/January/1};
    ASSERT_SAME_TYPE(decltype(d0.time_since_epoch()), days);
    assert( d0.time_since_epoch().count() == 0);
    assert( d1.time_since_epoch().count() == 10957);

    local_seconds s0{d0};
    local_seconds s1{d1};
    ASSERT_SAME_TYPE(decltype(s0.time_since_epoch()), seconds);
    assert( s0.time_since_epoch().count() == 0);
    assert( s1.time_since_epoch().count() == 946684800L);
}
