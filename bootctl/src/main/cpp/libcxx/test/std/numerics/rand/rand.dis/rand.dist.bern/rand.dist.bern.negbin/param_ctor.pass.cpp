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
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::negative_binomial_distribution<> D;
        typedef D::param_type param_type;
        param_type p;
        assert(p.k() == 1);
        assert(p.p() == 0.5);
    }
    {
        typedef std::negative_binomial_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10);
        assert(p.k() == 10);
        assert(p.p() == 0.5);
    }
    {
        typedef std::negative_binomial_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10, 0.25);
        assert(p.k() == 10);
        assert(p.p() == 0.25);
    }
}
