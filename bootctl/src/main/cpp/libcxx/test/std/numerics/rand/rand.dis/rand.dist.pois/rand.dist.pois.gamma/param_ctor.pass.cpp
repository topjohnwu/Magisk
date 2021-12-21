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
// class gamma_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::gamma_distribution<> D;
        typedef D::param_type param_type;
        param_type p;
        assert(p.alpha() == 1);
        assert(p.beta() == 1);
    }
    {
        typedef std::gamma_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10);
        assert(p.alpha() == 10);
        assert(p.beta() == 1);
    }
    {
        typedef std::gamma_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10, 5);
        assert(p.alpha() == 10);
        assert(p.beta() == 5);
    }
}
