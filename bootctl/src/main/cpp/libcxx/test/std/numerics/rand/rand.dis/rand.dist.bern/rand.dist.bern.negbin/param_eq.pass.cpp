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
        param_type p1(3, 0.75);
        param_type p2(3, 0.75);
        assert(p1 == p2);
    }
    {
        typedef std::negative_binomial_distribution<> D;
        typedef D::param_type param_type;
        param_type p1(3, 0.75);
        param_type p2(3, 0.5);
        assert(p1 != p2);
    }
}
