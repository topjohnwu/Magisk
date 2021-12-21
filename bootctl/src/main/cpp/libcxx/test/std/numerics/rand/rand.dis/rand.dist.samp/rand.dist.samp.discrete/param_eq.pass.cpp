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
// class discrete_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type param_type;
        double p0[] = {30, 10};
        param_type p1(p0, p0+2);
        param_type p2(p0, p0+2);
        assert(p1 == p2);
    }
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type param_type;
        double p0[] = {30, 10};
        param_type p1(p0, p0+2);
        param_type p2;
        assert(p1 != p2);
    }
}
