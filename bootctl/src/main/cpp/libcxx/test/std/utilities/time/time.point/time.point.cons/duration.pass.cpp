//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// time_point

// explicit time_point(const duration& d);

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::milliseconds Duration;
    {
    std::chrono::time_point<Clock, Duration> t(Duration(3));
    assert(t.time_since_epoch() == Duration(3));
    }
    {
    std::chrono::time_point<Clock, Duration> t(std::chrono::seconds(3));
    assert(t.time_since_epoch() == Duration(3000));
    }
#if TEST_STD_VER > 11
    {
    constexpr std::chrono::time_point<Clock, Duration> t(Duration(3));
    static_assert(t.time_since_epoch() == Duration(3), "");
    }
    {
    constexpr std::chrono::time_point<Clock, Duration> t(std::chrono::seconds(3));
    static_assert(t.time_since_epoch() == Duration(3000), "");
    }
#endif
}
