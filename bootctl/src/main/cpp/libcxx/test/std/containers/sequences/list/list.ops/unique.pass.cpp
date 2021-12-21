//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// void unique();

#include <list>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    int a1[] = {2, 1, 1, 4, 4, 4, 4, 3, 3};
    int a2[] = {2, 1, 4, 3};
    std::list<int> c(a1, a1+sizeof(a1)/sizeof(a1[0]));
    c.unique();
    assert(c == std::list<int>(a2, a2+4));
    }
#if TEST_STD_VER >= 11
    {
    int a1[] = {2, 1, 1, 4, 4, 4, 4, 3, 3};
    int a2[] = {2, 1, 4, 3};
    std::list<int, min_allocator<int>> c(a1, a1+sizeof(a1)/sizeof(a1[0]));
    c.unique();
    assert((c == std::list<int, min_allocator<int>>(a2, a2+4)));
    }
#endif
}
