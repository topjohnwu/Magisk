//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration

// duration& operator%=(const duration& rhs)

#include <chrono>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 14
constexpr bool test_constexpr()
{
    std::chrono::microseconds us1(11);
    std::chrono::microseconds us2(3);
    us1 %= us2;
    return us1.count() == 2;
}
#endif

int main()
{
    {
    std::chrono::microseconds us1(11);
    std::chrono::microseconds us2(3);
    us1 %= us2;
    assert(us1.count() == 2);
    us1 %= std::chrono::milliseconds(3);
    assert(us1.count() == 2);
    }

#if TEST_STD_VER > 14
    static_assert(test_constexpr(), "");
#endif
}
