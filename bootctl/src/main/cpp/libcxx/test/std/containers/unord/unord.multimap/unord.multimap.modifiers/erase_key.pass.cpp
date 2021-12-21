//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_multimap

// size_type erase(const key_type& k);

#include <unordered_map>
#include <string>
#include <cassert>
#include <cstddef>

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
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.erase(5) == 0);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        C::const_iterator k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 2);
        assert(k->second == "two");
        ++k;
        assert(k->first == 2);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(2) == 2);
        assert(c.size() == 4);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(2) == 0);
        assert(c.size() == 4);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(4) == 1);
        assert(c.size() == 3);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(4) == 0);
        assert(c.size() == 3);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(1) == 2);
        assert(c.size() == 1);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(1) == 0);
        assert(c.size() == 1);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(3) == 1);
        assert(c.size() == 0);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 0);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(3) == 0);
        assert(c.size() == 0);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 0);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.erase(5) == 0);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        C::const_iterator k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 2);
        assert(k->second == "two");
        ++k;
        assert(k->first == 2);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(2) == 2);
        assert(c.size() == 4);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(2) == 0);
        assert(c.size() == 4);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        eq = c.equal_range(4);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 4);
        assert(k->second == "four");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(4) == 1);
        assert(c.size() == 3);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(4) == 0);
        assert(c.size() == 3);
        eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
        assert(k->first == 1);
        assert(k->second == "one");
        ++k;
        assert(k->first == 1);
        assert(k->second == "four");
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(1) == 2);
        assert(c.size() == 1);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(1) == 0);
        assert(c.size() == 1);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 1);
        k = eq.first;
        assert(k->first == 3);
        assert(k->second == "three");
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(3) == 1);
        assert(c.size() == 0);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 0);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());

        assert(c.erase(3) == 0);
        assert(c.size() == 0);
        eq = c.equal_range(3);
        assert(std::distance(eq.first, eq.second) == 0);
        assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
        assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
    }
    {
    typedef std::unordered_multimap<int, int> C;
    C m, m2;
    for ( int i = 0; i < 10; ++i ) {
        for (int j = 0; j < 2; ++j ) {
            m.insert (std::make_pair(i,j));
            m2.insert(std::make_pair(i,j));
            }
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
