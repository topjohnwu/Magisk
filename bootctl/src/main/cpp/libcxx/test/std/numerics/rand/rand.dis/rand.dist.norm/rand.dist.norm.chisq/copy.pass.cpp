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
// class chi_squared_distribution

// chi_squared_distribution(const chi_squared_distribution&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::chi_squared_distribution<> D;
    D d1(21.75);
    D d2 = d1;
    assert(d1 == d2);
}

int main()
{
    test1();
}
