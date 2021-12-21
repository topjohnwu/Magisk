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

// explicit weibull_distribution(result_type a = 0, result_type b = 1);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::weibull_distribution<> D;
        D d;
        assert(d.a() == 1);
        assert(d.b() == 1);
    }
    {
        typedef std::weibull_distribution<> D;
        D d(14.5);
        assert(d.a() == 14.5);
        assert(d.b() == 1);
    }
    {
        typedef std::weibull_distribution<> D;
        D d(14.5, 5.25);
        assert(d.a() == 14.5);
        assert(d.b() == 5.25);
    }
}
