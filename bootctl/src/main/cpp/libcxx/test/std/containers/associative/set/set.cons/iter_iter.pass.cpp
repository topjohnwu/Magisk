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

// template <class InputIterator>
//     set(InputIterator first, InputIterator last);

#include <set>
#include <cassert>

#include "test_iterators.h"
#include "min_allocator.h"

int main()
{
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
    std::set<V> m(input_iterator<const int*>(ar),
                  input_iterator<const int*>(ar+sizeof(ar)/sizeof(ar[0])));
    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == 1);
    assert(*next(m.begin()) == 2);
    assert(*next(m.begin(), 2) == 3);
    }
#if TEST_STD_VER >= 11
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
    std::set<V, std::less<int>, min_allocator<int>> m(input_iterator<const int*>(ar),
                  input_iterator<const int*>(ar+sizeof(ar)/sizeof(ar[0])));
    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == 1);
    assert(*next(m.begin()) == 2);
    assert(*next(m.begin(), 2) == 3);
    }
#endif
}
