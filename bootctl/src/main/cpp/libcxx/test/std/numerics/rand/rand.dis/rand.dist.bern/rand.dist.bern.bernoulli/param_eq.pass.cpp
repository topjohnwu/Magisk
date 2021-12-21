//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// class bernoulli_distribution
// {
//     class param_type;

#include <random>
#include <limits>
#include <cassert>

int main()
{
    {
        typedef std::bernoulli_distribution D;
        typedef D::param_type param_type;
        param_type p1(0.75);
        param_type p2(0.75);
        assert(p1 == p2);
    }
    {
        typedef std::bernoulli_distribution D;
        typedef D::param_type param_type;
        param_type p1(0.75);
        param_type p2(0.5);
        assert(p1 != p2);
    }
}
