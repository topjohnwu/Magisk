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
// class exponential_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::exponential_distribution<> D;
        typedef D::param_type param_type;
        param_type p;
        assert(p.lambda() == 1);
    }
    {
        typedef std::exponential_distribution<> D;
        typedef D::param_type param_type;
        param_type p(10);
        assert(p.lambda() == 10);
    }
}
