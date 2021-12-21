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
// class cauchy_distribution

// bool operator=(const cauchy_distribution& x,
//                const cauchy_distribution& y);
// bool operator!(const cauchy_distribution& x,
//                const cauchy_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::cauchy_distribution<> D;
        D d1(2.5, 4);
        D d2(2.5, 4);
        assert(d1 == d2);
    }
    {
        typedef std::cauchy_distribution<> D;
        D d1(2.5, 4);
        D d2(2.5, 4.5);
        assert(d1 != d2);
    }
}
