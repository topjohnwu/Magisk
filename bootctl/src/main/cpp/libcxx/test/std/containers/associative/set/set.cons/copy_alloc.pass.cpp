//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class set

// set(const set& m, const allocator_type& a);

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
    std::set<int, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A(7));
    std::set<int, C, A> m(mo, A(3));
    assert(m.get_allocator() == A(3));
    assert(m.key_comp() == C(5));
    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == 1);
    assert(*next(m.begin()) == 2);
    assert(*next(m.begin(), 2) == 3);

    assert(mo.get_allocator() == A(7));
    assert(mo.key_comp() == C(5));
    assert(mo.size() == 3);
    assert(distance(mo.begin(), mo.end()) == 3);
    assert(*mo.begin() == 1);
    assert(*next(mo.begin()) == 2);
    assert(*next(mo.begin(), 2) == 3);
}
