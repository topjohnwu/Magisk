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

// template <class Rep2, class Period2>
//   duration(const duration<Rep2, Period2>& d);

//  conversions from integral to floating point durations allowed

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    std::chrono::duration<int> i(3);
    std::chrono::duration<double, std::milli> d = i;
    assert(d.count() == 3000);
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::chrono::duration<int> i(3);
    constexpr std::chrono::duration<double, std::milli> d = i;
    static_assert(d.count() == 3000, "");
    }
#endif
}
