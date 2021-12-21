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

// iterator erase(const_iterator first, const_iterator last)

#include <unordered_map>
#include <string>
#include <cassert>
#include <cstddef>

#include "min_allocator.h"

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
        C::const_iterator i = c.find(2);
        C::const_iterator j = next(i, 2);
        C::iterator k = c.erase(i, i);
        assert(k == i);
        assert(c.size() == 6);
        typedef std::pair<C::iterator, C::iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
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

        k = c.erase(i, j);
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

        k = c.erase(c.cbegin(), c.cend());
        assert(c.size() == 0);
        assert(k == c.end());
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
        C::const_iterator i = c.find(2);
        C::const_iterator j = next(i, 2);
        C::iterator k = c.erase(i, i);
        assert(k == i);
        assert(c.size() == 6);
        typedef std::pair<C::iterator, C::iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        k = eq.first;
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

        k = c.erase(i, j);
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

        k = c.erase(c.cbegin(), c.cend());
        assert(c.size() == 0);
        assert(k == c.end());
    }
#endif
}
