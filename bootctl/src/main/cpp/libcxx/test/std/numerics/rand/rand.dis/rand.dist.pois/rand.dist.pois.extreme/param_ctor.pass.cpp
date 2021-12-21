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
// class extreme_value_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::extreme_value_distribution<> D;
        typedef D::param_type param_type;
        param_type p;
        assert(p.a() == 0);
        assert(p.b() == 1);
    }
    {
        typedef std::extreme_value_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10);
        assert(p.a() == 10);
        assert(p.b() == 1);
    }
    {
        typedef std::extreme_value_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10, 5);
        assert(p.a() == 10);
        assert(p.b() == 5);
    }
}
