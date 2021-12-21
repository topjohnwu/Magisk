//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// void reverse();

#include <list>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    int a1[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    int a2[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    std::list<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
    c1.reverse();
    assert(c1 == std::list<int>(a2, a2+sizeof(a2)/sizeof(a2[0])));
    }
#if TEST_STD_VER >= 11
    {
    int a1[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    int a2[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    std::list<int, min_allocator<int>> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
    c1.reverse();
    assert((c1 == std::list<int, min_allocator<int>>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
    }
#endif
}
