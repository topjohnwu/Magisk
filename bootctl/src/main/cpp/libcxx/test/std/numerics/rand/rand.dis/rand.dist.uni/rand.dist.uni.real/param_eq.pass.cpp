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
// class uniform_real_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::uniform_real_distribution<float> D;
        typedef D::param_type param_type;
        param_type p1(5, 10);
        param_type p2(5, 10);
        assert(p1 == p2);
    }
    {
        typedef std::uniform_real_distribution<float> D;
        typedef D::param_type param_type;
        param_type p1(5, 10);
        param_type p2(6, 10);
        assert(p1 != p2);
    }
}
