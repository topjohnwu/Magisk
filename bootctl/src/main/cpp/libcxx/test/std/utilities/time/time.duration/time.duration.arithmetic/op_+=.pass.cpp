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

// constexpr duration& operator+=(const duration& d); // constexpr in C++17

#include <chrono>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 14
constexpr bool test_constexpr()
{
    std::chrono::seconds s(3);
    s += std::chrono::seconds(2);
    if (s.count() != 5) return false;
    s += std::chrono::minutes(2);
    return s.count() == 125;
}
#endif

int main()
{
    {
    std::chrono::seconds s(3);
    s += std::chrono::seconds(2);
    assert(s.count() == 5);
    s += std::chrono::minutes(2);
    assert(s.count() == 125);
    }

#if TEST_STD_VER > 14
    static_assert(test_constexpr(), "");
#endif
}
