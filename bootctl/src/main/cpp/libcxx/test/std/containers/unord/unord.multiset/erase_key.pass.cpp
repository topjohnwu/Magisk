//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_multiset

// size_type erase(const key_type& k);

#include <unordered_set>
#include <string>
#include <cassert>

#include "min_allocator.h"

#if TEST_STD_VER >= 11
template <typename Unordered>
bool only_deletions ( const Unordered &whole, const Unordered &part ) {
    typename Unordered::const_iterator w = whole.begin();
    typename Unordered::const_iterator p = part.begin();

    while ( w != whole.end () && p != part.end()) {
        if ( *w == *p )
            p++;
        w++;
        }

    return p == part.end();
}
#endif

int main()
{
    {
        typedef std::unordered_multiset<int> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.erase(5) == 0);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        assert(c.erase(2) == 2);
        assert(c.size() == 4);
        assert(c.count(1) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        assert(c.erase(2) == 0);
        assert(c.size() == 4);
        assert(c.count(1) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        assert(c.erase(4) == 1);
        assert(c.size() == 3);
        assert(c.count(1) == 2);
        assert(c.count(3) == 1);

        assert(c.erase(4) == 0);
        assert(c.size() == 3);
        assert(c.count(1) == 2);
        assert(c.count(3) == 1);

        assert(c.erase(1) == 2);
        assert(c.size() == 1);
        assert(c.count(3) == 1);

        assert(c.erase(1) == 0);
        assert(c.size() == 1);
        assert(c.count(3) == 1);

        assert(c.erase(3) == 1);
        assert(c.size() == 0);

        assert(c.erase(3) == 0);
        assert(c.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
                                      std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.erase(5) == 0);
        assert(c.size() == 6);
        assert(c.count(1) == 2);
        assert(c.count(2) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        assert(c.erase(2) == 2);
        assert(c.size() == 4);
        assert(c.count(1) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        assert(c.erase(2) == 0);
        assert(c.size() == 4);
        assert(c.count(1) == 2);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        assert(c.erase(4) == 1);
        assert(c.size() == 3);
        assert(c.count(1) == 2);
        assert(c.count(3) == 1);

        assert(c.erase(4) == 0);
        assert(c.size() == 3);
        assert(c.count(1) == 2);
        assert(c.count(3) == 1);

        assert(c.erase(1) == 2);
        assert(c.size() == 1);
        assert(c.count(3) == 1);

        assert(c.erase(1) == 0);
        assert(c.size() == 1);
        assert(c.count(3) == 1);

        assert(c.erase(3) == 1);
        assert(c.size() == 0);

        assert(c.erase(3) == 0);
        assert(c.size() == 0);
    }
    {
    typedef std::unordered_multiset<int> C;
    C m, m2;
    for ( int i = 0; i < 10; ++i ) {
        m.insert(i);  m.insert(i);
        m2.insert(i); m2.insert(i);
        }

    C::iterator i = m2.begin();
    int ctr = 0;
    while (i != m2.end()) {
        if (ctr++ % 2 == 0)
            m2.erase(i++);
        else
            ++i;
        }

    assert (only_deletions (m, m2));
    }
#endif
}
