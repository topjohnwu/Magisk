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

// template <class Duration2>
//   time_point(const time_point<clock, Duration2>& t);

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::microseconds Duration1;
    typedef std::chrono::milliseconds Duration2;
    {
    std::chrono::time_point<Clock, Duration2> t2(Duration2(3));
    std::chrono::time_point<Clock, Duration1> t1 = t2;
    assert(t1.time_since_epoch() == Duration1(3000));
    }
#if TEST_STD_VER > 11
    {
    constexpr std::chrono::time_point<Clock, Duration2> t2(Duration2(3));
    constexpr std::chrono::time_point<Clock, Duration1> t1 = t2;
    static_assert(t1.time_since_epoch() == Duration1(3000), "");
    }
#endif
}
