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
// class extreme_value_distribution

// extreme_value_distribution(const extreme_value_distribution&);

#include <random>
#include <cassert>

void
test1()
{
    typedef std::extreme_value_distribution<> D;
    D d1(.5, 1.75);
    D d2 = d1;
    assert(d1 == d2);
}

int main()
{
    test1();
}
