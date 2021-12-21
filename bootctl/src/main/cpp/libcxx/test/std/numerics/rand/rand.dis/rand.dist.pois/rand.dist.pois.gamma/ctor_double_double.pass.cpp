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

// explicit gamma_distribution(result_type alpha = 0, result_type beta = 1);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::gamma_distribution<> D;
        D d;
        assert(d.alpha() == 1);
        assert(d.beta() == 1);
    }
    {
        typedef std::gamma_distribution<> D;
        D d(14.5);
        assert(d.alpha() == 14.5);
        assert(d.beta() == 1);
    }
    {
        typedef std::gamma_distribution<> D;
        D d(14.5, 5.25);
        assert(d.alpha() == 14.5);
        assert(d.beta() == 5.25);
    }
}
