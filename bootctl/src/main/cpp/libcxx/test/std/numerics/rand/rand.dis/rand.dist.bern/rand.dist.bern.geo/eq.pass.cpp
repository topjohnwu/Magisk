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
// class geometric_distribution

// bool operator=(const geometric_distribution& x,
//                const geometric_distribution& y);
// bool operator!(const geometric_distribution& x,
//                const geometric_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::geometric_distribution<> D;
        D d1(.25);
        D d2(.25);
        assert(d1 == d2);
    }
    {
        typedef std::geometric_distribution<> D;
        D d1(.28);
        D d2(.25);
        assert(d1 != d2);
    }
}
