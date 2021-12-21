//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <set>

// class multiset

// multiset(multiset&& s);

#include <set>
#include <cassert>

#include "../../../test_compare.h"
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
        typedef int V;
        typedef test_compare<std::less<int> > C;
        typedef test_allocator<V> A;
        std::multiset<int, C, A> mo(C(5), A(7));
        std::multiset<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 0);
        assert(distance(m.begin(), m.end()) == 0);

        assert(mo.get_allocator() == A(test_alloc_base::moved_value));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(distance(mo.begin(), mo.end()) == 0);
    }
    {
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_compare<std::less<int> > C;
        typedef test_allocator<V> A;
        std::multiset<int, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A(7));
        std::multiset<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 9);
        assert(distance(m.begin(), m.end()) == 9);
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 1);
        assert(*next(m.begin(), 2) == 1);
        assert(*next(m.begin(), 3) == 2);
        assert(*next(m.begin(), 4) == 2);
        assert(*next(m.begin(), 5) == 2);
        assert(*next(m.begin(), 6) == 3);
        assert(*next(m.begin(), 7) == 3);
        assert(*next(m.begin(), 8) == 3);

        assert(mo.get_allocator() == A(test_alloc_base::moved_value));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(distance(mo.begin(), mo.end()) == 0);
    }
    {
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_compare<std::less<int> > C;
        typedef min_allocator<V> A;
        std::multiset<int, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A());
        std::multiset<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 9);
        assert(distance(m.begin(), m.end()) == 9);
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 1);
        assert(*next(m.begin(), 2) == 1);
        assert(*next(m.begin(), 3) == 2);
        assert(*next(m.begin(), 4) == 2);
        assert(*next(m.begin(), 5) == 2);
        assert(*next(m.begin(), 6) == 3);
        assert(*next(m.begin(), 7) == 3);
        assert(*next(m.begin(), 8) == 3);

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(distance(mo.begin(), mo.end()) == 0);
    }
}
