//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class IntType = int>
// class discrete_distribution

// discrete_distribution& operator=(const discrete_distribution&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::discrete_distribution<> D;
    double p[] = {2, 4, 1, 8};
    D d1(p, p+4);
    D d2;
    assert(d1 != d2);
    d2 = d1;
    assert(d1 == d2);
}

int main()
{
    test1();
}
