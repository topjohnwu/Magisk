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
// class normal_distribution

// explicit normal_distribution(result_type mean = 0, result_type stddev = 1);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::normal_distribution<> D;
        D d;
        assert(d.mean() == 0);
        assert(d.stddev() == 1);
    }
    {
        typedef std::normal_distribution<> D;
        D d(14.5);
        assert(d.mean() == 14.5);
        assert(d.stddev() == 1);
    }
    {
        typedef std::normal_distribution<> D;
        D d(14.5, 5.25);
        assert(d.mean() == 14.5);
        assert(d.stddev() == 5.25);
    }
}
