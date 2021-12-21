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
// class normal_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::normal_distribution<> D;
        typedef D::param_type param_type;
        param_type p;
        assert(p.mean() == 0);
        assert(p.stddev() == 1);
    }
    {
        typedef std::normal_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10);
        assert(p.mean() == 10);
        assert(p.stddev() == 1);
    }
    {
        typedef std::normal_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10, 5);
        assert(p.mean() == 10);
        assert(p.stddev() == 5);
    }
}
