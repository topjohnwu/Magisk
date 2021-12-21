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

// template <class Rep1, class Period1, class Rep2, class Period2>
//   constexpr
//   typename common_type<Rep1, Rep2>::type
//   operator/(const duration<Rep1, Period1>& lhs, const duration<Rep2, Period2>& rhs);

#include <chrono>
#include <cassert>

#include "test_macros.h"
#include "truncate_fp.h"

int main()
{
    {
    std::chrono::nanoseconds ns1(15);
    std::chrono::nanoseconds ns2(5);
    assert(ns1 / ns2 == 3);
    }
    {
    std::chrono::microseconds us1(15);
    std::chrono::nanoseconds ns2(5);
    assert(us1 / ns2 == 3000);
    }
    {
    std::chrono::duration<int, std::ratio<2, 3> > s1(30);
    std::chrono::duration<int, std::ratio<3, 5> > s2(5);
    assert(s1 / s2 == 6);
    }
    {
    std::chrono::duration<int, std::ratio<2, 3> > s1(30);
    std::chrono::duration<double, std::ratio<3, 5> > s2(5);
    assert(s1 / s2 == truncate_fp(20./3));
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::chrono::nanoseconds ns1(15);
    constexpr std::chrono::nanoseconds ns2(5);
    static_assert(ns1 / ns2 == 3, "");
    }
    {
    constexpr std::chrono::microseconds us1(15);
    constexpr std::chrono::nanoseconds ns2(5);
    static_assert(us1 / ns2 == 3000, "");
    }
    {
    constexpr std::chrono::duration<int, std::ratio<2, 3> > s1(30);
    constexpr std::chrono::duration<int, std::ratio<3, 5> > s2(5);
    static_assert(s1 / s2 == 6, "");
    }
    {
    constexpr std::chrono::duration<int, std::ratio<2, 3> > s1(30);
    constexpr std::chrono::duration<double, std::ratio<3, 5> > s2(5);
    static_assert(s1 / s2 == 20./3, "");
    }
#endif
}
