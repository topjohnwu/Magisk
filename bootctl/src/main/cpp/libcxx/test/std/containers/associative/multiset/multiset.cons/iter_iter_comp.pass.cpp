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

// template <class InputIterator>
//     multiset(InputIterator first, InputIterator last, const value_compare& comp);

#include <set>
#include <cassert>

#include "test_iterators.h"
#include "../../../test_compare.h"

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
    typedef test_compare<std::less<V> > C;
    std::multiset<V, C> m(input_iterator<const V*>(ar),
                     input_iterator<const V*>(ar+sizeof(ar)/sizeof(ar[0])), C(5));
    assert(m.value_comp() == C(5));
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
}
