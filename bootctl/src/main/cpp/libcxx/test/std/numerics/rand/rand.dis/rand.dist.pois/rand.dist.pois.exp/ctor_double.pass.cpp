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

// explicit exponential_distribution(RealType lambda = 1.0);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::exponential_distribution<> D;
        D d;
        assert(d.lambda() == 1);
    }
    {
        typedef std::exponential_distribution<> D;
        D d(3.5);
        assert(d.lambda() == 3.5);
    }
}
