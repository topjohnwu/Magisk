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

// explicit negative_binomial_distribution(IntType t = 1, double p = 0.5);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::negative_binomial_distribution<> D;
        D d;
        assert(d.k() == 1);
        assert(d.p() == 0.5);
    }
    {
        typedef std::negative_binomial_distribution<> D;
        D d(3);
        assert(d.k() == 3);
        assert(d.p() == 0.5);
    }
    {
        typedef std::negative_binomial_distribution<> D;
        D d(3, 0.75);
        assert(d.k() == 3);
        assert(d.p() == 0.75);
    }
}
