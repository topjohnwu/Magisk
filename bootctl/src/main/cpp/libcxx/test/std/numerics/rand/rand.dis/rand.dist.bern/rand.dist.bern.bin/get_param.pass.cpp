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

// param_type param() const;

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::binomial_distribution<> D;
        typedef D::param_type P;
        P p(5, .125);
        D d(p);
        assert(d.param() == p);
    }
}
