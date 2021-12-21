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
// class uniform_real_distribution

// bool operator=(const uniform_real_distribution& x,
//                const uniform_real_distribution& y);
// bool operator!(const uniform_real_distribution& x,
//                const uniform_real_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::uniform_real_distribution<> D;
        D d1(3, 8);
        D d2(3, 8);
        assert(d1 == d2);
    }
    {
        typedef std::uniform_real_distribution<> D;
        D d1(3, 8);
        D d2(3, 8.1);
        assert(d1 != d2);
    }
}
