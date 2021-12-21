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

// template <class Rep2>
//   explicit duration(const Rep2& r);

// construct double with int

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    std::chrono::duration<double> d(5);
    assert(d.count() == 5);
#if TEST_STD_VER >= 11
    constexpr std::chrono::duration<double> d2(5);
    static_assert(d2.count() == 5, "");
#endif
}
