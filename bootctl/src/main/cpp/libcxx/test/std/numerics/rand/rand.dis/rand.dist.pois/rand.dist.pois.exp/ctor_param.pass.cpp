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

// explicit exponential_distribution(const param_type& parm);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::exponential_distribution<> D;
        typedef D::param_type P;
        P p(0.25);
        D d(p);
        assert(d.lambda() == 0.25);
    }
}
