//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class multiset

// multiset();

#include <set>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    std::multiset<int> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
#if TEST_STD_VER >= 11
    {
    std::multiset<int, std::less<int>, min_allocator<int>> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
    {
    typedef explicit_allocator<int> A;
        {
        std::multiset<int, std::less<int>, A> m;
        assert(m.empty());
        assert(m.begin() == m.end());
        }
        {
        A a;
        std::multiset<int, std::less<int>, A> m(a);
        assert(m.empty());
        assert(m.begin() == m.end());
        }
    }
    {
    std::multiset<int> m = {};
    assert(m.empty());
    assert(m.begin() == m.end());
    }
#endif
}
