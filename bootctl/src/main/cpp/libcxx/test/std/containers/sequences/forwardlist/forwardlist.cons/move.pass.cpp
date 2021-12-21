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

// forward_list(forward_list&& x);

#include <forward_list>
#include <cassert>
#include <iterator>

#include "test_allocator.h"
#include "MoveOnly.h"
#include "min_allocator.h"

int main()
{
    {
        typedef MoveOnly T;
        typedef test_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t)), I(std::end(t)), A(10));
        C c = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c0.empty());
        assert(c.get_allocator() == A(10));
    }
    {
        typedef MoveOnly T;
        typedef other_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t)), I(std::end(t)), A(10));
        C c = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c0.empty());
        assert(c.get_allocator() == A(10));
    }
    {
        typedef MoveOnly T;
        typedef min_allocator<T> A;
        typedef std::forward_list<T, A> C;
        T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        typedef std::move_iterator<T*> I;
        C c0(I(std::begin(t)), I(std::end(t)), A());
        C c = std::move(c0);
        int n = 0;
        for (C::const_iterator i = c.begin(), e = c.end(); i != e; ++i, ++n)
            assert(*i == n);
        assert(n == std::end(t) - std::begin(t));
        assert(c0.empty());
        assert(c.get_allocator() == A());
    }
}
