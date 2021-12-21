//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test ratio_less_equal

#include <ratio>

#include "test_macros.h"

template <class Rat1, class Rat2, bool result>
void test()
{
    static_assert((result == std::ratio_less_equal<Rat1, Rat2>::value), "");
#if TEST_STD_VER > 14
    static_assert((result == std::ratio_less_equal_v<Rat1, Rat2>), "");
#endif
}

int main()
{
    {
    typedef std::ratio<1, 1> R1;
    typedef std::ratio<1, 1> R2;
    test<R1, R2, true>();
    }
    {
    typedef std::ratio<0x7FFFFFFFFFFFFFFFLL, 1> R1;
    typedef std::ratio<0x7FFFFFFFFFFFFFFFLL, 1> R2;
    test<R1, R2, true>();
    }
    {
    typedef std::ratio<-0x7FFFFFFFFFFFFFFFLL, 1> R1;
    typedef std::ratio<-0x7FFFFFFFFFFFFFFFLL, 1> R2;
    test<R1, R2, true>();
    }
    {
    typedef std::ratio<1, 0x7FFFFFFFFFFFFFFFLL> R1;
    typedef std::ratio<1, 0x7FFFFFFFFFFFFFFFLL> R2;
    test<R1, R2, true>();
    }
    {
    typedef std::ratio<1, 1> R1;
    typedef std::ratio<1, -1> R2;
    test<R1, R2, false>();
    }
    {
    typedef std::ratio<0x7FFFFFFFFFFFFFFFLL, 1> R1;
    typedef std::ratio<-0x7FFFFFFFFFFFFFFFLL, 1> R2;
    test<R1, R2, false>();
    }
    {
    typedef std::ratio<-0x7FFFFFFFFFFFFFFFLL, 1> R1;
    typedef std::ratio<0x7FFFFFFFFFFFFFFFLL, 1> R2;
    test<R1, R2, true>();
    }
    {
    typedef std::ratio<1, 0x7FFFFFFFFFFFFFFFLL> R1;
    typedef std::ratio<1, -0x7FFFFFFFFFFFFFFFLL> R2;
    test<R1, R2, false>();
    }
}
