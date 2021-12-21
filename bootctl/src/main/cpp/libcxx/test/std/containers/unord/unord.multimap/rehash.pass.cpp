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

// void rehash(size_type n);

#include <unordered_map>
#include <string>
#include <set>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstddef>

#include "test_macros.h"
#include "min_allocator.h"

template <class C>
void rehash_postcondition(const C& c, size_t n)
{
    assert(c.bucket_count() >= c.size() / c.max_load_factor() && c.bucket_count() >= n);
}

template <class C>
void test(const C& c)
{
    assert(c.size() == 6);
    typedef std::pair<typename C::const_iterator, typename C::const_iterator> Eq;
    Eq eq = c.equal_range(1);
    assert(std::distance(eq.first, eq.second) == 2);
    typename C::const_iterator i = eq.first;
    {
        std::set<std::string> s;
        s.insert("one");
        s.insert("four");
        for ( int n = 0; n < 2; ++n )
        {
            assert(i->first == 1);
            assert(s.find(i->second) != s.end());
            s.erase(s.find(i->second));
            ++i;
        }
    }
    eq = c.equal_range(2);
    assert(std::distance(eq.first, eq.second) == 2);
    i = eq.first;
    {
        std::set<std::string> s;
        s.insert("two");
        s.insert("four");
        for ( int n = 0; n < 2; ++n )
        {
            assert(i->first == 2);
            assert(s.find(i->second) != s.end());
            s.erase(s.find(i->second));
            ++i;
        }
    }
    eq = c.equal_range(3);
    assert(std::distance(eq.first, eq.second) == 1);
    i = eq.first;
    assert(i->first == 3);
    assert(i->second == "three");
    eq = c.equal_range(4);
    assert(std::distance(eq.first, eq.second) == 1);
    i = eq.first;
    assert(i->first == 4);
    assert(i->second == "four");
    assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
    assert(static_cast<std::size_t>(std::distance(c.cbegin(), c.cend())) == c.size());
    assert(std::fabs(c.load_factor() - (float)c.size()/c.bucket_count()) < FLT_EPSILON);
}

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
        test(c);
        assert(c.bucket_count() >= 7);
        c.rehash(3);
        rehash_postcondition(c, 3);
        LIBCPP_ASSERT(c.bucket_count() == 7);
        test(c);
        c.max_load_factor(2);
        c.rehash(3);
        rehash_postcondition(c, 3);
        LIBCPP_ASSERT(c.bucket_count() == 3);
        test(c);
        c.rehash(31);
        rehash_postcondition(c, 31);
        LIBCPP_ASSERT(c.bucket_count() == 31);
        test(c);
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
        test(c);
        assert(c.bucket_count() >= 7);
        c.rehash(3);
        rehash_postcondition(c, 3);
        LIBCPP_ASSERT(c.bucket_count() == 7);
        test(c);
        c.max_load_factor(2);
        c.rehash(3);
        rehash_postcondition(c, 3);
        LIBCPP_ASSERT(c.bucket_count() == 3);
        test(c);
        c.rehash(31);
        rehash_postcondition(c, 31);
        LIBCPP_ASSERT(c.bucket_count() == 31);
        test(c);
    }
#endif
}
