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

// time_point();

#include <chrono>
#include <cassert>

#include "test_macros.h"
#include "../../rep.h"

int main()
{
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::duration<Rep, std::milli> Duration;
    {
    std::chrono::time_point<Clock, Duration> t;
    assert(t.time_since_epoch() == Duration::zero());
    }
#if TEST_STD_VER > 11
    {
    constexpr std::chrono::time_point<Clock, Duration> t;
    static_assert(t.time_since_epoch() == Duration::zero(), "");
    }
#endif
}
