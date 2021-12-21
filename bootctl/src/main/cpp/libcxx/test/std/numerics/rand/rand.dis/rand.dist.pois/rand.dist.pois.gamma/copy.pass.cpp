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
// class gamma_distribution

// gamma_distribution(const gamma_distribution&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::gamma_distribution<> D;
    D d1(20, 1.75);
    D d2 = d1;
    assert(d1 == d2);
}

int main()
{
    test1();
}
