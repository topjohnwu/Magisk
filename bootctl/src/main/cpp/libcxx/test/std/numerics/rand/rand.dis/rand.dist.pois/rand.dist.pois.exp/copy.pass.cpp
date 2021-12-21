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
// class exponential_distribution

// exponential_distribution(const exponential_distribution&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::exponential_distribution<> D;
    D d1(1.75);
    D d2 = d1;
    assert(d1 == d2);
}

int main()
{
    test1();
}
