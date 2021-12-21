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
// class weibull_distribution

// result_type max() const;

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::weibull_distribution<> D;
        D d(5, .25);
        D::result_type m = d.max();
        assert(m == INFINITY);
    }
}
