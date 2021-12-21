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
// class gamma_distribution

// explicit gamma_distribution(const param_type& parm);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::gamma_distribution<> D;
        typedef D::param_type P;
        P p(0.25, 10);
        D d(p);
        assert(d.alpha() == 0.25);
        assert(d.beta() == 10);
    }
}
