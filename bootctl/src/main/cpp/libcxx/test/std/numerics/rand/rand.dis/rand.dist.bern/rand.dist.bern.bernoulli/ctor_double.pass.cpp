//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// class bernoulli_distribution

// explicit bernoulli_distribution(double p = 0.5);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::bernoulli_distribution D;
        D d;
        assert(d.p() == 0.5);
    }
    {
        typedef std::bernoulli_distribution D;
        D d(0);
        assert(d.p() == 0);
    }
    {
        typedef std::bernoulli_distribution D;
        D d(0.75);
        assert(d.p() == 0.75);
    }
}
