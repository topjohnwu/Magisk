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

// pair<const_iterator, const_iterator> equal_range(const key_type& k) const;

#include <unordered_map>
#include <string>
#include <set>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef C::const_iterator I;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fiftyA"),
            P(50, "fiftyB"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c(std::begin(a), std::end(a));
        std::pair<I, I> r = c.equal_range(30);
        assert(std::distance(r.first, r.second) == 1);
        assert(r.first->first == 30);
        assert(r.first->second == "thirty");
        r = c.equal_range(5);
        assert(std::distance(r.first, r.second) == 0);
        r = c.equal_range(50);
        std::set<std::string> s;
        s.insert("fifty");
        s.insert("fiftyA");
        s.insert("fiftyB");
        for ( int i = 0; i < 3; ++i )
        {
            assert(r.first->first == 50);
            assert(s.find(r.first->second) != s.end());
            s.erase(s.find(r.first->second));
            ++r.first;
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef C::const_iterator I;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fiftyA"),
            P(50, "fiftyB"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c(std::begin(a), std::end(a));
        std::pair<I, I> r = c.equal_range(30);
        assert(std::distance(r.first, r.second) == 1);
        assert(r.first->first == 30);
        assert(r.first->second == "thirty");
        r = c.equal_range(5);
        assert(std::distance(r.first, r.second) == 0);
        r = c.equal_range(50);
        std::set<std::string> s;
        s.insert("fifty");
        s.insert("fiftyA");
        s.insert("fiftyB");
        for ( int i = 0; i < 3; ++i )
        {
            assert(r.first->first == 50);
            assert(s.find(r.first->second) != s.end());
            s.erase(s.find(r.first->second));
            ++r.first;
        }
    }
#endif
}
