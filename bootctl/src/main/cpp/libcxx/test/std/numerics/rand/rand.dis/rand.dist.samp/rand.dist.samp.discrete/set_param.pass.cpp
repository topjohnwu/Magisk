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

// void param(const param_type& parm);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::discrete_distribution<> D;
        typedef D::param_type P;
        double d0[] = {.3, .1, .6};
        P p(d0, d0+3);
        D d;
        d.param(p);
        assert(d.param() == p);
    }
}
