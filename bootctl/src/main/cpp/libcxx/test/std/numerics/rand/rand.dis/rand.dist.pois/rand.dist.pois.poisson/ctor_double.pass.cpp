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
// class poisson_distribution

// explicit poisson_distribution(RealType lambda = 1.0);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::poisson_distribution<> D;
        D d;
        assert(d.mean() == 1);
    }
    {
        typedef std::poisson_distribution<> D;
        D d(3.5);
        assert(d.mean() == 3.5);
    }
}
