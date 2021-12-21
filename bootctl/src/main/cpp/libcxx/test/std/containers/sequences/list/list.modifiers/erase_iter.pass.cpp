//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// iterator erase(const_iterator position);

#include <list>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    int a1[] = {1, 2, 3};
    std::list<int> l1(a1, a1+3);
    std::list<int>::const_iterator i = l1.begin();
    ++i;
    std::list<int>::iterator j = l1.erase(i);
    assert(l1.size() == 2);
    assert(distance(l1.begin(), l1.end()) == 2);
    assert(*j == 3);
    assert(*l1.begin() == 1);
    assert(*next(l1.begin()) == 3);
    j = l1.erase(j);
    assert(j == l1.end());
    assert(l1.size() == 1);
    assert(distance(l1.begin(), l1.end()) == 1);
    assert(*l1.begin() == 1);
    j = l1.erase(l1.begin());
    assert(j == l1.end());
    assert(l1.size() == 0);
    assert(distance(l1.begin(), l1.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
    int a1[] = {1, 2, 3};
    std::list<int, min_allocator<int>> l1(a1, a1+3);
    std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
    ++i;
    std::list<int, min_allocator<int>>::iterator j = l1.erase(i);
    assert(l1.size() == 2);
    assert(distance(l1.begin(), l1.end()) == 2);
    assert(*j == 3);
    assert(*l1.begin() == 1);
    assert(*next(l1.begin()) == 3);
    j = l1.erase(j);
    assert(j == l1.end());
    assert(l1.size() == 1);
    assert(distance(l1.begin(), l1.end()) == 1);
    assert(*l1.begin() == 1);
    j = l1.erase(l1.begin());
    assert(j == l1.end());
    assert(l1.size() == 0);
    assert(distance(l1.begin(), l1.end()) == 0);
    }
#endif
}
