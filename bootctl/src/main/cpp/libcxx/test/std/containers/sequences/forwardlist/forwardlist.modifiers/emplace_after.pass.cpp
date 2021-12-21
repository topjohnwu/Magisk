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

// template <class... Args>
//     iterator emplace_after(const_iterator p, Args&&... args);

#include <forward_list>
#include <cassert>

#include "../../../Emplaceable.h"
#include "min_allocator.h"

int main()
{
    {
        typedef Emplaceable T;
        typedef std::forward_list<T> C;
        typedef C::iterator I;
        C c;
        I i = c.emplace_after(c.cbefore_begin());
        assert(i == c.begin());
        assert(c.front() == Emplaceable());
        assert(distance(c.begin(), c.end()) == 1);

        i = c.emplace_after(c.cbegin(), 1, 2.5);
        assert(i == next(c.begin()));
        assert(c.front() == Emplaceable());
        assert(*next(c.begin()) == Emplaceable(1, 2.5));
        assert(distance(c.begin(), c.end()) == 2);

        i = c.emplace_after(next(c.cbegin()), 2, 3.5);
        assert(i == next(c.begin(), 2));
        assert(c.front() == Emplaceable());
        assert(*next(c.begin()) == Emplaceable(1, 2.5));
        assert(*next(c.begin(), 2) == Emplaceable(2, 3.5));
        assert(distance(c.begin(), c.end()) == 3);

        i = c.emplace_after(c.cbegin(), 3, 4.5);
        assert(i == next(c.begin()));
        assert(c.front() == Emplaceable());
        assert(*next(c.begin(), 1) == Emplaceable(3, 4.5));
        assert(*next(c.begin(), 2) == Emplaceable(1, 2.5));
        assert(*next(c.begin(), 3) == Emplaceable(2, 3.5));
        assert(distance(c.begin(), c.end()) == 4);
    }
    {
        typedef Emplaceable T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef C::iterator I;
        C c;
        I i = c.emplace_after(c.cbefore_begin());
        assert(i == c.begin());
        assert(c.front() == Emplaceable());
        assert(distance(c.begin(), c.end()) == 1);

        i = c.emplace_after(c.cbegin(), 1, 2.5);
        assert(i == next(c.begin()));
        assert(c.front() == Emplaceable());
        assert(*next(c.begin()) == Emplaceable(1, 2.5));
        assert(distance(c.begin(), c.end()) == 2);

        i = c.emplace_after(next(c.cbegin()), 2, 3.5);
        assert(i == next(c.begin(), 2));
        assert(c.front() == Emplaceable());
        assert(*next(c.begin()) == Emplaceable(1, 2.5));
        assert(*next(c.begin(), 2) == Emplaceable(2, 3.5));
        assert(distance(c.begin(), c.end()) == 3);

        i = c.emplace_after(c.cbegin(), 3, 4.5);
        assert(i == next(c.begin()));
        assert(c.front() == Emplaceable());
        assert(*next(c.begin(), 1) == Emplaceable(3, 4.5));
        assert(*next(c.begin(), 2) == Emplaceable(1, 2.5));
        assert(*next(c.begin(), 3) == Emplaceable(2, 3.5));
        assert(distance(c.begin(), c.end()) == 4);
    }
}
