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

// explicit lognormal_distribution(result_type mean = 0, result_type stddev = 1);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::lognormal_distribution<> D;
        D d;
        assert(d.m() == 0);
        assert(d.s() == 1);
    }
    {
        typedef std::lognormal_distribution<> D;
        D d(14.5);
        assert(d.m() == 14.5);
        assert(d.s() == 1);
    }
    {
        typedef std::lognormal_distribution<> D;
        D d(14.5, 5.25);
        assert(d.m() == 14.5);
        assert(d.s() == 5.25);
    }
}
