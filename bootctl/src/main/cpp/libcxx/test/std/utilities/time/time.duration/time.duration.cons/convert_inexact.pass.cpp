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

// inexact conversions allowed for floating point reps

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    std::chrono::duration<double, std::micro> us(1);
    std::chrono::duration<double, std::milli> ms = us;
    assert(ms.count() == 1./1000);
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::chrono::duration<double, std::micro> us(1);
    constexpr std::chrono::duration<double, std::milli> ms = us;
    static_assert(ms.count() == 1./1000, "");
    }
#endif
}
