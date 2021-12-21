//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class RealType = double>
// class piecewise_constant_distribution

// piecewise_constant_distribution& operator=(const piecewise_constant_distribution&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::piecewise_constant_distribution<> D;
    double p[] = {2, 4, 1, 8};
    double b[] = {2, 4, 5, 8, 9};
    D d1(b, b+5, p);
    D d2;
    assert(d1 != d2);
    d2 = d1;
    assert(d1 == d2);
}

int main()
{
    test1();
}
