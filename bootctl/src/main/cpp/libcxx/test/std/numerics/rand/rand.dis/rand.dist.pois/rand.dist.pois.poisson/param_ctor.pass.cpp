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
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::poisson_distribution<> D;
        typedef D::param_type param_type;
        param_type p;
        assert(p.mean() == 1);
    }
    {
        typedef std::poisson_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10);
        assert(p.mean() == 10);
    }
}
