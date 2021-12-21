//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// void push_front(const value_type& v);

#include <forward_list>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        C c;
        c.push_front(1);
        assert(c.front() == 1);
        assert(distance(c.begin(), c.end()) == 1);
        c.push_front(3);
        assert(c.front() == 3);
        assert(*next(c.begin()) == 1);
        assert(distance(c.begin(), c.end()) == 2);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c;
        c.push_front(1);
        assert(c.front() == 1);
        assert(distance(c.begin(), c.end()) == 1);
        c.push_front(3);
        assert(c.front() == 3);
        assert(*next(c.begin()) == 1);
        assert(distance(c.begin(), c.end()) == 2);
    }
#endif
}
