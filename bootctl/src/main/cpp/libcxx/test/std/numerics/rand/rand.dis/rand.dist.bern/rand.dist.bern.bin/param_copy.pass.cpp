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
// class binomial_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::binomial_distribution<> D;
        typedef D::param_type param_type;
        param_type p0(10, .125);
        param_type p = p0;
        assert(p.t() == 10);
        assert(p.p() == .125);
    }
}
