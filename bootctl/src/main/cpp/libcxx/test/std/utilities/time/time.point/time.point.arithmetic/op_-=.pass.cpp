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

// time_point& operator-=(const duration& d);
// constexpr in c++17

#include <chrono>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 14
constexpr bool constexpr_test()
{
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::milliseconds Duration;
    std::chrono::time_point<Clock, Duration> t(Duration(5));
    t -= Duration(4);
    return t.time_since_epoch() == Duration(1);
}
#endif

int main()
{
    {
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::milliseconds Duration;
    std::chrono::time_point<Clock, Duration> t(Duration(3));
    t -= Duration(2);
    assert(t.time_since_epoch() == Duration(1));
    }

#if TEST_STD_VER > 14
    static_assert(constexpr_test(), "");
#endif
}
