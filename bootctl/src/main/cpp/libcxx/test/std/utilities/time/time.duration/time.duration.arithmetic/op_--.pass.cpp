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

// constexpr duration& operator--();  // constexpr in C++17

#include <chrono>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 14
constexpr bool test_constexpr()
{
    std::chrono::hours h(3);
    return (--h).count() == 2;
}
#endif

int main()
{
    {
    std::chrono::hours h(3);
    std::chrono::hours& href = --h;
    assert(&href == &h);
    assert(h.count() == 2);
    }

#if TEST_STD_VER > 14
    static_assert(test_constexpr(), "");
#endif
}
