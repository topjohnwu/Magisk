//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// map();

#include <map>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    std::map<int, double> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
#if TEST_STD_VER >= 11
    {
    std::map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
    {
    typedef explicit_allocator<std::pair<const int, double>> A;
        {
        std::map<int, double, std::less<int>, A> m;
        assert(m.empty());
        assert(m.begin() == m.end());
        }
        {
        A a;
        std::map<int, double, std::less<int>, A> m(a);
        assert(m.empty());
        assert(m.begin() == m.end());
        }
    }
    {
    std::map<int, double> m = {};
    assert(m.empty());
    assert(m.begin() == m.end());
    }
#endif
}
