//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test ratio:  The static data members num and den shall have the common
//    divisor of the absolute values of N and D:

#include <ratio>

template <long long N, long long D, long long eN, long long eD>
void test()
{
    static_assert((std::ratio<N, D>::num == eN), "");
    static_assert((std::ratio<N, D>::den == eD), "");
}

int main()
{
    test<1, 1, 1, 1>();
    test<1, 10, 1, 10>();
    test<10, 10, 1, 1>();
    test<10, 1, 10, 1>();
    test<12, 4, 3, 1>();
    test<12, -4, -3, 1>();
    test<-12, 4, -3, 1>();
    test<-12, -4, 3, 1>();
    test<4, 12, 1, 3>();
    test<4, -12, -1, 3>();
    test<-4, 12, -1, 3>();
    test<-4, -12, 1, 3>();
    test<222, 333, 2, 3>();
    test<222, -333, -2, 3>();
    test<-222, 333, -2, 3>();
    test<-222, -333, 2, 3>();
    test<0x7FFFFFFFFFFFFFFFLL, 127, 72624976668147841LL, 1>();
    test<-0x7FFFFFFFFFFFFFFFLL, 127, -72624976668147841LL, 1>();
    test<0x7FFFFFFFFFFFFFFFLL, -127, -72624976668147841LL, 1>();
    test<-0x7FFFFFFFFFFFFFFFLL, -127, 72624976668147841LL, 1>();
}
