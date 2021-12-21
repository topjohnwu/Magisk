//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// template <class InputIterator>
//     iterator insert_after(const_iterator p,
//                           InputIterator first, InputIterator last);

#include <forward_list>
#include <cassert>

#include "test_iterators.h"
#include "min_allocator.h"

int main()
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        typedef C::iterator I;
        typedef input_iterator<const T*> J;
        C c;
        const T t[] = {0, 1, 2, 3, 4};
        I i = c.insert_after(c.cbefore_begin(), J(t), J(t));
        assert(i == c.before_begin());
        assert(distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), J(t), J(t+3));
        assert(i == next(c.before_begin(), 3));
        assert(distance(c.begin(), c.end()) == 3);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 2);

        i = c.insert_after(c.begin(), J(t+3), J(t+5));
        assert(i == next(c.begin(), 2));
        assert(distance(c.begin(), c.end()) == 5);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 3);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 1);
        assert(*next(c.begin(), 4) == 2);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        typedef C::iterator I;
        typedef input_iterator<const T*> J;
        C c;
        const T t[] = {0, 1, 2, 3, 4};
        I i = c.insert_after(c.cbefore_begin(), J(t), J(t));
        assert(i == c.before_begin());
        assert(distance(c.begin(), c.end()) == 0);

        i = c.insert_after(c.cbefore_begin(), J(t), J(t+3));
        assert(i == next(c.before_begin(), 3));
        assert(distance(c.begin(), c.end()) == 3);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 2);

        i = c.insert_after(c.begin(), J(t+3), J(t+5));
        assert(i == next(c.begin(), 2));
        assert(distance(c.begin(), c.end()) == 5);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 3);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 1);
        assert(*next(c.begin(), 4) == 2);
    }
#endif
}
