//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// iterator erase_after(const_iterator first, const_iterator last);

#include <forward_list>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef int T;
        typedef std::forward_list<T> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));

        C::iterator i = c.erase_after(next(c.cbefore_begin(), 4), next(c.cbefore_begin(), 4));
        assert(i == next(c.cbefore_begin(), 4));
        assert(distance(c.begin(), c.end()) == 10);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 2);
        assert(*next(c.begin(), 3) == 3);
        assert(*next(c.begin(), 4) == 4);
        assert(*next(c.begin(), 5) == 5);
        assert(*next(c.begin(), 6) == 6);
        assert(*next(c.begin(), 7) == 7);
        assert(*next(c.begin(), 8) == 8);
        assert(*next(c.begin(), 9) == 9);

        i = c.erase_after(next(c.cbefore_begin(), 2), next(c.cbefore_begin(), 5));
        assert(i == next(c.begin(), 2));
        assert(distance(c.begin(), c.end()) == 8);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 5);
        assert(*next(c.begin(), 4) == 6);
        assert(*next(c.begin(), 5) == 7);
        assert(*next(c.begin(), 6) == 8);
        assert(*next(c.begin(), 7) == 9);

        i = c.erase_after(next(c.cbefore_begin(), 2), next(c.cbefore_begin(), 3));
        assert(i == next(c.begin(), 2));
        assert(distance(c.begin(), c.end()) == 8);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 5);
        assert(*next(c.begin(), 4) == 6);
        assert(*next(c.begin(), 5) == 7);
        assert(*next(c.begin(), 6) == 8);
        assert(*next(c.begin(), 7) == 9);

        i = c.erase_after(next(c.cbefore_begin(), 5), next(c.cbefore_begin(), 9));
        assert(i == c.end());
        assert(distance(c.begin(), c.end()) == 5);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 5);
        assert(*next(c.begin(), 4) == 6);

        i = c.erase_after(next(c.cbefore_begin(), 0), next(c.cbefore_begin(), 2));
        assert(i == c.begin());
        assert(distance(c.begin(), c.end()) == 4);
        assert(*next(c.begin(), 0) == 1);
        assert(*next(c.begin(), 1) == 4);
        assert(*next(c.begin(), 2) == 5);
        assert(*next(c.begin(), 3) == 6);

        i = c.erase_after(next(c.cbefore_begin(), 0), next(c.cbefore_begin(), 5));
        assert(i == c.begin());
        assert(i == c.end());
        assert(distance(c.begin(), c.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));

        C::iterator i = c.erase_after(next(c.cbefore_begin(), 4), next(c.cbefore_begin(), 4));
        assert(i == next(c.cbefore_begin(), 4));
        assert(distance(c.begin(), c.end()) == 10);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 2);
        assert(*next(c.begin(), 3) == 3);
        assert(*next(c.begin(), 4) == 4);
        assert(*next(c.begin(), 5) == 5);
        assert(*next(c.begin(), 6) == 6);
        assert(*next(c.begin(), 7) == 7);
        assert(*next(c.begin(), 8) == 8);
        assert(*next(c.begin(), 9) == 9);

        i = c.erase_after(next(c.cbefore_begin(), 2), next(c.cbefore_begin(), 5));
        assert(i == next(c.begin(), 2));
        assert(distance(c.begin(), c.end()) == 8);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 5);
        assert(*next(c.begin(), 4) == 6);
        assert(*next(c.begin(), 5) == 7);
        assert(*next(c.begin(), 6) == 8);
        assert(*next(c.begin(), 7) == 9);

        i = c.erase_after(next(c.cbefore_begin(), 2), next(c.cbefore_begin(), 3));
        assert(i == next(c.begin(), 2));
        assert(distance(c.begin(), c.end()) == 8);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 5);
        assert(*next(c.begin(), 4) == 6);
        assert(*next(c.begin(), 5) == 7);
        assert(*next(c.begin(), 6) == 8);
        assert(*next(c.begin(), 7) == 9);

        i = c.erase_after(next(c.cbefore_begin(), 5), next(c.cbefore_begin(), 9));
        assert(i == c.end());
        assert(distance(c.begin(), c.end()) == 5);
        assert(*next(c.begin(), 0) == 0);
        assert(*next(c.begin(), 1) == 1);
        assert(*next(c.begin(), 2) == 4);
        assert(*next(c.begin(), 3) == 5);
        assert(*next(c.begin(), 4) == 6);

        i = c.erase_after(next(c.cbefore_begin(), 0), next(c.cbefore_begin(), 2));
        assert(i == c.begin());
        assert(distance(c.begin(), c.end()) == 4);
        assert(*next(c.begin(), 0) == 1);
        assert(*next(c.begin(), 1) == 4);
        assert(*next(c.begin(), 2) == 5);
        assert(*next(c.begin(), 3) == 6);

        i = c.erase_after(next(c.cbefore_begin(), 0), next(c.cbefore_begin(), 5));
        assert(i == c.begin());
        assert(i == c.end());
        assert(distance(c.begin(), c.end()) == 0);
    }
#endif
}
