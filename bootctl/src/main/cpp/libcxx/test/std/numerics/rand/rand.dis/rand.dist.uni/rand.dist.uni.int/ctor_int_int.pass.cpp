//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class _IntType = int>
// class uniform_int_distribution

// explicit uniform_int_distribution(IntType a = 0,
//                                   IntType b = numeric_limits<IntType>::max());

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::uniform_int_distribution<> D;
        D d;
        assert(d.a() == 0);
        assert(d.b() == std::numeric_limits<int>::max());
    }
    {
        typedef std::uniform_int_distribution<> D;
        D d(-6);
        assert(d.a() == -6);
        assert(d.b() == std::numeric_limits<int>::max());
    }
    {
        typedef std::uniform_int_distribution<> D;
        D d(-6, 106);
        assert(d.a() == -6);
        assert(d.b() == 106);
    }
}
