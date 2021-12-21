//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// void pop_front();

#include <list>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    int a[] = {1, 2, 3};
    std::list<int> c(a, a+3);
    c.pop_front();
    assert(c == std::list<int>(a+1, a+3));
    c.pop_front();
    assert(c == std::list<int>(a+2, a+3));
    c.pop_front();
    assert(c.empty());
    }
#if TEST_STD_VER >= 11
    {
    int a[] = {1, 2, 3};
    std::list<int, min_allocator<int>> c(a, a+3);
    c.pop_front();
    assert((c == std::list<int, min_allocator<int>>(a+1, a+3)));
    c.pop_front();
    assert((c == std::list<int, min_allocator<int>>(a+2, a+3)));
    c.pop_front();
    assert(c.empty());
    }
#endif
}
