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
// class extreme_value_distribution

// param_type param() const;

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::extreme_value_distribution<> D;
        typedef D::param_type P;
        P p(.125, .5);
        D d(p);
        assert(d.param() == p);
    }
}
