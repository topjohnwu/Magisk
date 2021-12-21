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
// class fisher_f_distribution

// explicit fisher_f_distribution(result_type alpha = 0, result_type beta = 1);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::fisher_f_distribution<> D;
        D d;
        assert(d.m() == 1);
        assert(d.n() == 1);
    }
    {
        typedef std::fisher_f_distribution<> D;
        D d(14.5);
        assert(d.m() == 14.5);
        assert(d.n() == 1);
    }
    {
        typedef std::fisher_f_distribution<> D;
        D d(14.5, 5.25);
        assert(d.m() == 14.5);
        assert(d.n() == 5.25);
    }
}
