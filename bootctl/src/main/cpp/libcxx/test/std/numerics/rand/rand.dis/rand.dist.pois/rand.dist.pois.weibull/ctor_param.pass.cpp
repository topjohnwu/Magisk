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

// explicit weibull_distribution(const param_type& parm);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::weibull_distribution<> D;
        typedef D::param_type P;
        P p(0.25, 10);
        D d(p);
        assert(d.a() == 0.25);
        assert(d.b() == 10);
    }
}
