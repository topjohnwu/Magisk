//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// void splice(const_iterator position, list& x);

#include <list>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    int a1[] = {1, 2, 3};
    int a2[] = {4, 5, 6};
    {
        std::list<int> l1;
        std::list<int> l2;
        l1.splice(l1.end(), l2);
        assert(l1.size() == 0);
        assert(distance(l1.begin(), l1.end()) == 0);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
    }
    {
        std::list<int> l1;
        std::list<int> l2(a2, a2+1);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 1);
        assert(distance(l1.begin(), l1.end()) == 1);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 4);
    }
    {
        std::list<int> l1;
        std::list<int> l2(a2, a2+2);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
    }
    {
        std::list<int> l1;
        std::list<int> l2(a2, a2+3);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
    }
    {
        std::list<int> l1(a1, a1+1);
        std::list<int> l2;
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 1);
        assert(distance(l1.begin(), l1.end()) == 1);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
    }
    {
        std::list<int> l1(a1, a1+1);
        std::list<int> l2;
        l1.splice(l1.end(), l2);
        assert(l1.size() == 1);
        assert(distance(l1.begin(), l1.end()) == 1);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
    }
    {
        std::list<int> l1(a1, a1+1);
        std::list<int> l2(a2, a2+1);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 1);
    }
    {
        std::list<int> l1(a1, a1+1);
        std::list<int> l2(a2, a2+1);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
    }
    {
        std::list<int> l1(a1, a1+1);
        std::list<int> l2(a2, a2+2);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 1);
    }
    {
        std::list<int> l1(a1, a1+1);
        std::list<int> l2(a2, a2+2);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
    }
    {
        std::list<int> l1(a1, a1+1);
        std::list<int> l2(a2, a2+3);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
        ++i;
        assert(*i == 1);
    }
    {
        std::list<int> l1(a1, a1+1);
        std::list<int> l2(a2, a2+3);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
    }
    {
        std::list<int> l1(a1, a1+2);
        std::list<int> l2;
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int> l1(a1, a1+2);
        std::list<int> l2;
        l1.splice(next(l1.begin()), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int> l1(a1, a1+2);
        std::list<int> l2;
        l1.splice(next(l1.begin(), 2), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int> l1(a1, a1+2);
        std::list<int> l2(a2, a2+1);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int> l1(a1, a1+2);
        std::list<int> l2(a2, a2+1);
        l1.splice(next(l1.begin()), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int> l1(a1, a1+2);
        std::list<int> l2(a2, a2+1);
        l1.splice(next(l1.begin(), 2), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 4);
    }
    {
        std::list<int> l1(a1, a1+2);
        std::list<int> l2(a2, a2+2);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int> l1(a1, a1+2);
        std::list<int> l2(a2, a2+2);
        l1.splice(next(l1.begin()), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int> l1(a1, a1+2);
        std::list<int> l2(a2, a2+2);
        l1.splice(next(l1.begin(), 2), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
    }
    {
        std::list<int> l1(a1, a1+3);
        std::list<int> l2(a2, a2+3);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 6);
        assert(distance(l1.begin(), l1.end()) == 6);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
        ++i;
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 3);
    }
    {
        std::list<int> l1(a1, a1+3);
        std::list<int> l2(a2, a2+3);
        l1.splice(next(l1.begin()), l2);
        assert(l1.size() == 6);
        assert(distance(l1.begin(), l1.end()) == 6);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 3);
    }
    {
        std::list<int> l1(a1, a1+3);
        std::list<int> l2(a2, a2+3);
        l1.splice(next(l1.begin(), 2), l2);
        assert(l1.size() == 6);
        assert(distance(l1.begin(), l1.end()) == 6);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
        ++i;
        assert(*i == 3);
    }
    {
        std::list<int> l1(a1, a1+3);
        std::list<int> l2(a2, a2+3);
        l1.splice(next(l1.begin(), 3), l2);
        assert(l1.size() == 6);
        assert(distance(l1.begin(), l1.end()) == 6);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 3);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
    }
#if TEST_STD_VER >= 11
    {
        std::list<int, min_allocator<int>> l1;
        std::list<int, min_allocator<int>> l2;
        l1.splice(l1.end(), l2);
        assert(l1.size() == 0);
        assert(distance(l1.begin(), l1.end()) == 0);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
    }
    {
        std::list<int, min_allocator<int>> l1;
        std::list<int, min_allocator<int>> l2(a2, a2+1);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 1);
        assert(distance(l1.begin(), l1.end()) == 1);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 4);
    }
    {
        std::list<int, min_allocator<int>> l1;
        std::list<int, min_allocator<int>> l2(a2, a2+2);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
    }
    {
        std::list<int, min_allocator<int>> l1;
        std::list<int, min_allocator<int>> l2(a2, a2+3);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+1);
        std::list<int, min_allocator<int>> l2;
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 1);
        assert(distance(l1.begin(), l1.end()) == 1);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+1);
        std::list<int, min_allocator<int>> l2;
        l1.splice(l1.end(), l2);
        assert(l1.size() == 1);
        assert(distance(l1.begin(), l1.end()) == 1);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+1);
        std::list<int, min_allocator<int>> l2(a2, a2+1);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 1);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+1);
        std::list<int, min_allocator<int>> l2(a2, a2+1);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+1);
        std::list<int, min_allocator<int>> l2(a2, a2+2);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 1);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+1);
        std::list<int, min_allocator<int>> l2(a2, a2+2);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+1);
        std::list<int, min_allocator<int>> l2(a2, a2+3);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
        ++i;
        assert(*i == 1);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+1);
        std::list<int, min_allocator<int>> l2(a2, a2+3);
        l1.splice(l1.end(), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+2);
        std::list<int, min_allocator<int>> l2;
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+2);
        std::list<int, min_allocator<int>> l2;
        l1.splice(next(l1.begin()), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+2);
        std::list<int, min_allocator<int>> l2;
        l1.splice(next(l1.begin(), 2), l2);
        assert(l1.size() == 2);
        assert(distance(l1.begin(), l1.end()) == 2);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+2);
        std::list<int, min_allocator<int>> l2(a2, a2+1);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+2);
        std::list<int, min_allocator<int>> l2(a2, a2+1);
        l1.splice(next(l1.begin()), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+2);
        std::list<int, min_allocator<int>> l2(a2, a2+1);
        l1.splice(next(l1.begin(), 2), l2);
        assert(l1.size() == 3);
        assert(distance(l1.begin(), l1.end()) == 3);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 4);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+2);
        std::list<int, min_allocator<int>> l2(a2, a2+2);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 1);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+2);
        std::list<int, min_allocator<int>> l2(a2, a2+2);
        l1.splice(next(l1.begin()), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 2);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+2);
        std::list<int, min_allocator<int>> l2(a2, a2+2);
        l1.splice(next(l1.begin(), 2), l2);
        assert(l1.size() == 4);
        assert(distance(l1.begin(), l1.end()) == 4);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+3);
        std::list<int, min_allocator<int>> l2(a2, a2+3);
        l1.splice(l1.begin(), l2);
        assert(l1.size() == 6);
        assert(distance(l1.begin(), l1.end()) == 6);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
        ++i;
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 3);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+3);
        std::list<int, min_allocator<int>> l2(a2, a2+3);
        l1.splice(next(l1.begin()), l2);
        assert(l1.size() == 6);
        assert(distance(l1.begin(), l1.end()) == 6);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 3);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+3);
        std::list<int, min_allocator<int>> l2(a2, a2+3);
        l1.splice(next(l1.begin(), 2), l2);
        assert(l1.size() == 6);
        assert(distance(l1.begin(), l1.end()) == 6);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
        ++i;
        assert(*i == 3);
    }
    {
        std::list<int, min_allocator<int>> l1(a1, a1+3);
        std::list<int, min_allocator<int>> l2(a2, a2+3);
        l1.splice(next(l1.begin(), 3), l2);
        assert(l1.size() == 6);
        assert(distance(l1.begin(), l1.end()) == 6);
        assert(l2.size() == 0);
        assert(distance(l2.begin(), l2.end()) == 0);
        std::list<int, min_allocator<int>>::const_iterator i = l1.begin();
        assert(*i == 1);
        ++i;
        assert(*i == 2);
        ++i;
        assert(*i == 3);
        ++i;
        assert(*i == 4);
        ++i;
        assert(*i == 5);
        ++i;
        assert(*i == 6);
    }
#endif
}
