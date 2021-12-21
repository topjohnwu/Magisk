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
// class exponential_distribution

// bool operator=(const exponential_distribution& x,
//                const exponential_distribution& y);
// bool operator!(const exponential_distribution& x,
//                const exponential_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::exponential_distribution<> D;
        D d1(.25);
        D d2(.25);
        assert(d1 == d2);
    }
    {
        typedef std::exponential_distribution<> D;
        D d1(.28);
        D d2(.25);
        assert(d1 != d2);
    }
}
