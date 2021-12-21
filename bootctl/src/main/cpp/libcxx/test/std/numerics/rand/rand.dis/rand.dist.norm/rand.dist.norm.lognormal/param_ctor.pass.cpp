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
// class lognormal_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::lognormal_distribution<> D;
        typedef D::param_type param_type;
        param_type p;
        assert(p.m() == 0);
        assert(p.s() == 1);
    }
    {
        typedef std::lognormal_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10);
        assert(p.m() == 10);
        assert(p.s() == 1);
    }
    {
        typedef std::lognormal_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10, 5);
        assert(p.m() == 10);
        assert(p.s() == 5);
    }
}
