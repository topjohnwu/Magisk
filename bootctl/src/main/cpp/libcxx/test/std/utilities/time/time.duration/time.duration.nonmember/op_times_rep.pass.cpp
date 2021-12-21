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
//   operator*(const duration<Rep1, Period>& d, const Rep2& s);

// template <class Rep1, class Period, class Rep2>
//   constexpr
//   duration<typename common_type<Rep1, Rep2>::type, Period>
//   operator*(const Rep1& s, const duration<Rep2, Period>& d);

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    std::chrono::nanoseconds ns(3);
    ns = ns * 5;
    assert(ns.count() == 15);
    ns = 6 * ns;
    assert(ns.count() == 90);
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::chrono::nanoseconds ns(3);
    constexpr std::chrono::nanoseconds ns2 = ns * 5;
    static_assert(ns2.count() == 15, "");
    constexpr std::chrono::nanoseconds ns3 = 6 * ns;
    static_assert(ns3.count() == 18, "");
    }
#endif
}
