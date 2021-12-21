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

// bool operator=(const negative_binomial_distribution& x,
//                const negative_binomial_distribution& y);
// bool operator!(const negative_binomial_distribution& x,
//                const negative_binomial_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::negative_binomial_distribution<> D;
        D d1(3, .25);
        D d2(3, .25);
        assert(d1 == d2);
    }
    {
        typedef std::negative_binomial_distribution<> D;
        D d1(3, .28);
        D d2(3, .25);
        assert(d1 != d2);
    }
    {
        typedef std::negative_binomial_distribution<> D;
        D d1(3, .25);
        D d2(4, .25);
        assert(d1 != d2);
    }
}
