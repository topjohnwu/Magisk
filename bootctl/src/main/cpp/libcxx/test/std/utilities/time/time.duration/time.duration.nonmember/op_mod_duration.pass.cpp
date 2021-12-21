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
//   typename common_type<duration<Rep1, Period1>, duration<Rep2, Period2>>::type
//   operator%(const duration<Rep1, Period1>& lhs, const duration<Rep2, Period2>& rhs);

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    std::chrono::nanoseconds ns1(15);
    std::chrono::nanoseconds ns2(6);
    std::chrono::nanoseconds r = ns1 % ns2;
    assert(r.count() == 3);
    }
    {
    std::chrono::microseconds us1(15);
    std::chrono::nanoseconds ns2(28);
    std::chrono::nanoseconds r = us1 % ns2;
    assert(r.count() == 20);
    }
    {
    std::chrono::duration<int, std::ratio<3, 5> > s1(6);
    std::chrono::duration<int, std::ratio<2, 3> > s2(3);
    std::chrono::duration<int, std::ratio<1, 15> > r = s1 % s2;
    assert(r.count() == 24);
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::chrono::nanoseconds ns1(15);
    constexpr std::chrono::nanoseconds ns2(6);
    constexpr std::chrono::nanoseconds r = ns1 % ns2;
    static_assert(r.count() == 3, "");
    }
    {
    constexpr std::chrono::microseconds us1(15);
    constexpr std::chrono::nanoseconds ns2(28);
    constexpr std::chrono::nanoseconds r = us1 % ns2;
    static_assert(r.count() == 20, "");
    }
    {
    constexpr std::chrono::duration<int, std::ratio<3, 5> > s1(6);
    constexpr std::chrono::duration<int, std::ratio<2, 3> > s2(3);
    constexpr std::chrono::duration<int, std::ratio<1, 15> > r = s1 % s2;
    static_assert(r.count() == 24, "");
    }
#endif
}
