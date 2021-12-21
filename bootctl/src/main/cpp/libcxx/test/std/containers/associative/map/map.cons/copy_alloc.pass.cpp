//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// map(const map& m, const allocator_type& a);

#include <map>
#include <cassert>

#include "../../../test_compare.h"
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::pair<const int, double> V;
    V ar[] =
    {
        V(1, 1),
        V(1, 1.5),
        V(1, 2),
        V(2, 1),
        V(2, 1.5),
        V(2, 2),
        V(3, 1),
        V(3, 1.5),
        V(3, 2),
    };
    typedef test_compare<std::less<int> > C;
    typedef test_allocator<V> A;
    std::map<int, double, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A(7));
    std::map<int, double, C, A> m(mo, A(3));
    assert(m.get_allocator() == A(3));
    assert(m.key_comp() == C(5));
    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == V(1, 1));
    assert(*next(m.begin()) == V(2, 1));
    assert(*next(m.begin(), 2) == V(3, 1));

    assert(mo.get_allocator() == A(7));
    assert(mo.key_comp() == C(5));
    assert(mo.size() == 3);
    assert(distance(mo.begin(), mo.end()) == 3);
    assert(*mo.begin() == V(1, 1));
    assert(*next(mo.begin()) == V(2, 1));
    assert(*next(mo.begin(), 2) == V(3, 1));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::pair<const int, double> V;
    V ar[] =
    {
        V(1, 1),
        V(1, 1.5),
        V(1, 2),
        V(2, 1),
        V(2, 1.5),
        V(2, 2),
        V(3, 1),
        V(3, 1.5),
        V(3, 2),
    };
    typedef test_compare<std::less<int> > C;
    typedef min_allocator<V> A;
    std::map<int, double, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A());
    std::map<int, double, C, A> m(mo, A());
    assert(m.get_allocator() == A());
    assert(m.key_comp() == C(5));
    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == V(1, 1));
    assert(*next(m.begin()) == V(2, 1));
    assert(*next(m.begin(), 2) == V(3, 1));

    assert(mo.get_allocator() == A());
    assert(mo.key_comp() == C(5));
    assert(mo.size() == 3);
    assert(distance(mo.begin(), mo.end()) == 3);
    assert(*mo.begin() == V(1, 1));
    assert(*next(mo.begin()) == V(2, 1));
    assert(*next(mo.begin(), 2) == V(3, 1));
    }
    {
    typedef std::pair<const int, double> V;
    V ar[] =
    {
        V(1, 1),
        V(1, 1.5),
        V(1, 2),
        V(2, 1),
        V(2, 1.5),
        V(2, 2),
        V(3, 1),
        V(3, 1.5),
        V(3, 2),
    };
    typedef test_compare<std::less<int> > C;
    typedef explicit_allocator<V> A;
    std::map<int, double, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A{});
    std::map<int, double, C, A> m(mo, A{});
    assert(m.get_allocator() == A());
    assert(m.key_comp() == C(5));
    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == V(1, 1));
    assert(*next(m.begin()) == V(2, 1));
    assert(*next(m.begin(), 2) == V(3, 1));

    assert(mo.get_allocator() == A());
    assert(mo.key_comp() == C(5));
    assert(mo.size() == 3);
    assert(distance(mo.begin(), mo.end()) == 3);
    assert(*mo.begin() == V(1, 1));
    assert(*next(mo.begin()) == V(2, 1));
    assert(*next(mo.begin(), 2) == V(3, 1));
    }
#endif
}
