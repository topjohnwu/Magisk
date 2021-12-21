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
// class negative_binomial_distribution

// negative_binomial_distribution(const negative_binomial_distribution&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::negative_binomial_distribution<> D;
    D d1(2, 0.75);
    D d2 = d1;
    assert(d1 == d2);
}

int main()
{
    test1();
}
