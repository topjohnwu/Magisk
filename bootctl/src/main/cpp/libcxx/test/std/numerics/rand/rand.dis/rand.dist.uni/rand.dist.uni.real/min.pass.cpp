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

// result_type min() const;

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::uniform_real_distribution<> D;
        D d(3, 8);
        assert(d.min() == 3);
    }
}
