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
//   bool
//   operator==(const duration<Rep1, Period1>& lhs, const duration<Rep2, Period2>& rhs);

// template <class Rep1, class Period1, class Rep2, class Period2>
//   constexpr
//   bool
//   operator!=(const duration<Rep1, Period1>& lhs, const duration<Rep2, Period2>& rhs);

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    std::chrono::seconds s1(3);
    std::chrono::seconds s2(3);
    assert(s1 == s2);
    assert(!(s1 != s2));
    }
    {
    std::chrono::seconds s1(3);
    std::chrono::seconds s2(4);
    assert(!(s1 == s2));
    assert(s1 != s2);
    }
    {
    std::chrono::milliseconds s1(3);
    std::chrono::microseconds s2(3000);
    assert(s1 == s2);
    assert(!(s1 != s2));
    }
    {
    std::chrono::milliseconds s1(3);
    std::chrono::microseconds s2(4000);
    assert(!(s1 == s2));
    assert(s1 != s2);
    }
    {
    std::chrono::duration<int, std::ratio<2, 3> > s1(9);
    std::chrono::duration<int, std::ratio<3, 5> > s2(10);
    assert(s1 == s2);
    assert(!(s1 != s2));
    }
    {
    std::chrono::duration<int, std::ratio<2, 3> > s1(10);
    std::chrono::duration<int, std::ratio<3, 5> > s2(9);
    assert(!(s1 == s2));
    assert(s1 != s2);
    }
    {
    std::chrono::duration<int, std::ratio<2, 3> > s1(9);
    std::chrono::duration<double, std::ratio<3, 5> > s2(10);
    assert(s1 == s2);
    assert(!(s1 != s2));
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::chrono::seconds s1(3);
    constexpr std::chrono::seconds s2(3);
    static_assert(s1 == s2, "");
    static_assert(!(s1 != s2), "");
    }
    {
    constexpr std::chrono::seconds s1(3);
    constexpr std::chrono::seconds s2(4);
    static_assert(!(s1 == s2), "");
    static_assert(s1 != s2, "");
    }
    {
    constexpr std::chrono::milliseconds s1(3);
    constexpr std::chrono::microseconds s2(3000);
    static_assert(s1 == s2, "");
    static_assert(!(s1 != s2), "");
    }
    {
    constexpr std::chrono::milliseconds s1(3);
    constexpr std::chrono::microseconds s2(4000);
    static_assert(!(s1 == s2), "");
    static_assert(s1 != s2, "");
    }
    {
    constexpr std::chrono::duration<int, std::ratio<2, 3> > s1(9);
    constexpr std::chrono::duration<int, std::ratio<3, 5> > s2(10);
    static_assert(s1 == s2, "");
    static_assert(!(s1 != s2), "");
    }
    {
    constexpr std::chrono::duration<int, std::ratio<2, 3> > s1(10);
    constexpr std::chrono::duration<int, std::ratio<3, 5> > s2(9);
    static_assert(!(s1 == s2), "");
    static_assert(s1 != s2, "");
    }
    {
    constexpr std::chrono::duration<int, std::ratio<2, 3> > s1(9);
    constexpr std::chrono::duration<double, std::ratio<3, 5> > s2(10);
    static_assert(s1 == s2, "");
    static_assert(!(s1 != s2), "");
    }
#endif
}
