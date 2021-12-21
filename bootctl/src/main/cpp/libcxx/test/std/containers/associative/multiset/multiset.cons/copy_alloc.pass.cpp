//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class multiset

// multiset(const multiset& m, const allocator_type& a);

#include <set>
#include <cassert>

#include "../../../test_compare.h"
#include "test_allocator.h"

int main()
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
    std::multiset<int, C, A> m(mo, A(3));
    assert(m.get_allocator() == A(3));
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

    assert(mo.get_allocator() == A(7));
    assert(mo.key_comp() == C(5));
    assert(mo.size() == 9);
    assert(distance(mo.begin(), mo.end()) == 9);
    assert(*next(mo.begin(), 0) == 1);
    assert(*next(mo.begin(), 1) == 1);
    assert(*next(mo.begin(), 2) == 1);
    assert(*next(mo.begin(), 3) == 2);
    assert(*next(mo.begin(), 4) == 2);
    assert(*next(mo.begin(), 5) == 2);
    assert(*next(mo.begin(), 6) == 3);
    assert(*next(mo.begin(), 7) == 3);
    assert(*next(mo.begin(), 8) == 3);
}
