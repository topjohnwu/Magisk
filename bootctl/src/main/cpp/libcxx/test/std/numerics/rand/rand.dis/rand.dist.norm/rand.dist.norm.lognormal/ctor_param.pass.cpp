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
// class lognormal_distribution

// explicit lognormal_distribution(const param_type& parm);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::lognormal_distribution<> D;
        typedef D::param_type P;
        P p(0.25, 10);
        D d(p);
        assert(d.m() == 0.25);
        assert(d.s() == 10);
    }
}
