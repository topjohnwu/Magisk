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

// bool operator=(const fisher_f_distribution& x,
//                const fisher_f_distribution& y);
// bool operator!(const fisher_f_distribution& x,
//                const fisher_f_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::fisher_f_distribution<> D;
        D d1(2.5, 4);
        D d2(2.5, 4);
        assert(d1 == d2);
    }
    {
        typedef std::fisher_f_distribution<> D;
        D d1(2.5, 4);
        D d2(2.5, 4.5);
        assert(d1 != d2);
    }
}
