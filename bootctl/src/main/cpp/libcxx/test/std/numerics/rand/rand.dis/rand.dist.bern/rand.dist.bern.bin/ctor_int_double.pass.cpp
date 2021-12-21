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
// class binomial_distribution

// explicit binomial_distribution(IntType t = 1, double p = 0.5);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::binomial_distribution<> D;
        D d;
        assert(d.t() == 1);
        assert(d.p() == 0.5);
    }
    {
        typedef std::binomial_distribution<> D;
        D d(3);
        assert(d.t() == 3);
        assert(d.p() == 0.5);
    }
    {
        typedef std::binomial_distribution<> D;
        D d(3, 0.75);
        assert(d.t() == 3);
        assert(d.p() == 0.75);
    }
}
