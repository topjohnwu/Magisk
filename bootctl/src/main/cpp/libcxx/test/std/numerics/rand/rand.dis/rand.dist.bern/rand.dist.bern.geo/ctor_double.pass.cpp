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
// class geometric_distribution

// explicit geometric_distribution(double p = 0.5);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::geometric_distribution<> D;
        D d;
        assert(d.p() == 0.5);
    }
    {
        typedef std::geometric_distribution<> D;
        D d(0.75);
        assert(d.p() == 0.75);
    }
}
