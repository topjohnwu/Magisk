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
// class lognormal_distribution

// lognormal_distribution(const lognormal_distribution&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::lognormal_distribution<> D;
    D d1(20, 1.75);
    D d2 = d1;
    assert(d1 == d2);
}

int main()
{
    test1();
}
