//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// template <class Compare> void merge(list& x, Compare comp);
// If (&addressof(x) == this) does nothing; otherwise ...

#include <list>
#include <functional>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    int a1[] = {10, 9, 7, 3, 1};
    int a2[] = {11, 8, 6, 5, 4, 2, 0};
    int a3[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    std::list<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
    std::list<int> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
    c1.merge(c2, std::greater<int>());
    assert(c1 == std::list<int>(a3, a3+sizeof(a3)/sizeof(a3[0])));
    assert(c2.empty());
    }
    {
    int a1[] = {10, 9, 7, 3, 1};
    std::list<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
    c1.merge(c1, std::greater<int>());
    assert((c1 == std::list<int>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
    }

#if TEST_STD_VER >= 11
    {
    int a1[] = {10, 9, 7, 3, 1};
    int a2[] = {11, 8, 6, 5, 4, 2, 0};
    int a3[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    std::list<int, min_allocator<int>> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
    std::list<int, min_allocator<int>> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
    c1.merge(c2, std::greater<int>());
    assert((c1 == std::list<int, min_allocator<int>>(a3, a3+sizeof(a3)/sizeof(a3[0]))));
    assert(c2.empty());
    }
#endif
}
