//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// forward_list(const forward_list& x, const allocator_type& a);

#include <forward_list>
#include <cassert>
#include <iterator>

#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
        typedef int T;
        typedef test_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t), std::end(t), A(10));
        C c(c0, A(9));
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c == c0);
        assert(c.get_allocator() == A(9));
    }
    {
        typedef int T;
        typedef other_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t), std::end(t), A(10));
        C c(c0, A(9));
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c == c0);
        assert(c.get_allocator() == A(9));
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef min_allocator<int> A;
        typedef std::forward_list<T, A> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c0(std::begin(t), std::end(t), A());
        C c(c0, A());
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c == c0);
        assert(c.get_allocator() == A());
    }
#endif
}
