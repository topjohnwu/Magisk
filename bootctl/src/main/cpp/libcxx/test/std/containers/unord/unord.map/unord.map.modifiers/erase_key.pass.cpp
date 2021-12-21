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
// class unordered_map

// size_type erase(const key_type& k);

#include <unordered_map>
#include <string>
#include <cassert>

#include "test_macros.h"
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
        typedef std::unordered_map<int, std::string> C;
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(2) == 1);
        assert(c.size() == 3);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(2) == 0);
        assert(c.size() == 3);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(4) == 1);
        assert(c.size() == 2);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");

        assert(c.erase(4) == 0);
        assert(c.size() == 2);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");

        assert(c.erase(1) == 1);
        assert(c.size() == 1);
        assert(c.at(3) == "three");

        assert(c.erase(1) == 0);
        assert(c.size() == 1);
        assert(c.at(3) == "three");

        assert(c.erase(3) == 1);
        assert(c.size() == 0);

        assert(c.erase(3) == 0);
        assert(c.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
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
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(2) == 1);
        assert(c.size() == 3);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(2) == 0);
        assert(c.size() == 3);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");

        assert(c.erase(4) == 1);
        assert(c.size() == 2);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");

        assert(c.erase(4) == 0);
        assert(c.size() == 2);
        assert(c.at(1) == "one");
        assert(c.at(3) == "three");

        assert(c.erase(1) == 1);
        assert(c.size() == 1);
        assert(c.at(3) == "three");

        assert(c.erase(1) == 0);
        assert(c.size() == 1);
        assert(c.at(3) == "three");

        assert(c.erase(3) == 1);
        assert(c.size() == 0);

        assert(c.erase(3) == 0);
        assert(c.size() == 0);
    }
    {
    typedef std::unordered_map<int, int> C;
    C m, m2;
    for ( int i = 0; i < 10; ++i ) {
        m[i] = i;
        m2[i] = i;
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
