//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <forward_list>

// iterator insert_after(const_iterator p, initializer_list<value_type> il);

#include <forward_list>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), {});
        assert(i == c.before_begin());
        assert(distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), {0, 1, 2});
        assert(i == next(c.before_begin(), 3));
        assert(distance(c.begin(), c.end()) == 3);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 2);

        i = c.insert_after(c.begin(), {3, 4});
        assert(i == next(c.begin(), 2));
        assert(distance(c.begin(), c.end()) == 5);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 3);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 1);
        assert(*next(c.begin(), 4) == 2);
    }
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef C::iterator I;
        C c;
        I i = c.insert_after(c.cbefore_begin(), {});
        assert(i == c.before_begin());
        assert(distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), {0, 1, 2});
        assert(i == next(c.before_begin(), 3));
        assert(distance(c.begin(), c.end()) == 3);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 2);

        i = c.insert_after(c.begin(), {3, 4});
        assert(i == next(c.begin(), 2));
        assert(distance(c.begin(), c.end()) == 5);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 3);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 1);
        assert(*next(c.begin(), 4) == 2);
    }
}
