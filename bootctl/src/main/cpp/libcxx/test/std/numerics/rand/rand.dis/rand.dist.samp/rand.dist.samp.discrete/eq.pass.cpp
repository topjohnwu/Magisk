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

// bool operator=(const discrete_distribution& x,
//                const discrete_distribution& y);
// bool operator!(const discrete_distribution& x,
//                const discrete_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::discrete_distribution<> D;
        D d1;
        D d2;
        assert(d1 == d2);
    }
    {
        typedef std::discrete_distribution<> D;
        double p0[] = {1};
        D d1(p0, p0+1);
        D d2;
        assert(d1 == d2);
    }
    {
        typedef std::discrete_distribution<> D;
        double p0[] = {10, 30};
        D d1(p0, p0+2);
        D d2;
        assert(d1 != d2);
    }
}
