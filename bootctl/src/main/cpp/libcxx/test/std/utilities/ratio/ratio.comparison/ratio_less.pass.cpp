//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test ratio_less

#include <ratio>

#include "test_macros.h"

template <class Rat1, class Rat2, bool result>
void test()
{
    static_assert((result == std::ratio_less<Rat1, Rat2>::value), "");
#if TEST_STD_VER > 14
    static_assert((result == std::ratio_less_v<Rat1, Rat2>), "");
#endif
}

int main()
{
    {
    typedef std::ratio<1, 1> R1;
    typedef std::ratio<1, 1> R2;
    test<R1, R2, false>();
    }
    {
    typedef std::ratio<0x7FFFFFFFFFFFFFFFLL, 1> R1;
    typedef std::ratio<0x7FFFFFFFFFFFFFFFLL, 1> R2;
    test<R1, R2, false>();
    }
    {
    typedef std::ratio<-0x7FFFFFFFFFFFFFFFLL, 1> R1;
    typedef std::ratio<-0x7FFFFFFFFFFFFFFFLL, 1> R2;
    test<R1, R2, false>();
    }
    {
    typedef std::ratio<1, 0x7FFFFFFFFFFFFFFFLL> R1;
    typedef std::ratio<1, 0x7FFFFFFFFFFFFFFFLL> R2;
    test<R1, R2, false>();
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
    {
    typedef std::ratio<0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFELL> R1;
    typedef std::ratio<0x7FFFFFFFFFFFFFFDLL, 0x7FFFFFFFFFFFFFFCLL> R2;
    test<R1, R2, true>();
    }
    {
    typedef std::ratio<0x7FFFFFFFFFFFFFFDLL, 0x7FFFFFFFFFFFFFFCLL> R1;
    typedef std::ratio<0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFELL> R2;
    test<R1, R2, false>();
    }
    {
    typedef std::ratio<-0x7FFFFFFFFFFFFFFDLL, 0x7FFFFFFFFFFFFFFCLL> R1;
    typedef std::ratio<-0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFELL> R2;
    test<R1, R2, true>();
    }
    {
    typedef std::ratio<0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFELL> R1;
    typedef std::ratio<0x7FFFFFFFFFFFFFFELL, 0x7FFFFFFFFFFFFFFDLL> R2;
    test<R1, R2, true>();
    }
    {
    typedef std::ratio<641981, 1339063> R1;
    typedef std::ratio<1291640, 2694141LL> R2;
    test<R1, R2, false>();
    }
    {
    typedef std::ratio<1291640, 2694141LL> R1;
    typedef std::ratio<641981, 1339063> R2;
    test<R1, R2, true>();
    }
}
