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
// class piecewise_constant_distribution

// bool operator=(const piecewise_constant_distribution& x,
//                const piecewise_constant_distribution& y);
// bool operator!(const piecewise_constant_distribution& x,
//                const piecewise_constant_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::piecewise_constant_distribution<> D;
        D d1;
        D d2;
        assert(d1 == d2);
    }
    {
        typedef std::piecewise_constant_distribution<> D;
        double b[] = {10, 14, 16, 17};
        double p[] = {25, 62.5, 12.5};
        D d1(b, b+4, p);
        D d2(b, b+4, p);
        assert(d1 == d2);
    }
    {
        typedef std::piecewise_constant_distribution<> D;
        double b[] = {10, 14, 16, 17};
        double p[] = {25, 62.5, 12.5};
        D d1(b, b+4, p);
        D d2;
        assert(d1 != d2);
    }
}
