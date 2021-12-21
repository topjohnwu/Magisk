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

// template <class Rep1, class Period, class Rep2>
//   constexpr
//   duration<typename common_type<Rep1, Rep2>::type, Period>
//   operator/(const duration<Rep1, Period>& d, const Rep2& s);

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    std::chrono::nanoseconds ns(15);
    ns = ns / 5;
    assert(ns.count() == 3);
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::chrono::nanoseconds ns(15);
    constexpr std::chrono::nanoseconds ns2 = ns / 5;
    static_assert(ns2.count() == 3, "");
    }
#endif
}
