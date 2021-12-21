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

// template <class InputIterator>
//     unordered_multimap(InputIterator first, InputIterator last, size_type n,
//                   const hasher& hf);

#include <unordered_map>
#include <string>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstddef>

#include "test_macros.h"
#include "test_iterators.h"
#include "../../../NotConstructible.h"
#include "../../../test_compare.h"
#include "../../../test_hash.h"
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_multimap<int, std::string,
                                   test_hash<std::hash<int> >,
                                   test_compare<std::equal_to<int> >,
                                   test_allocator<std::pair<const int, std::string> >
                                   > C;
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
        C c(input_iterator<P*>(a), input_iterator<P*>(a + sizeof(a)/sizeof(a[0])),
            7,
            test_hash<std::hash<int> >(8)
           );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        C::const_iterator i = eq.first;
        assert(i->first == 1);
        assert(i->second == "one");
        ++i;
        assert(i->first == 1);
        assert(i->second == "four");
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        i = eq.first;
        assert(i->first == 2);
        assert(i->second == "two");
        ++i;
        assert(i->first == 2);
        assert(i->second == "four");

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
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<std::hash<int> >(8));
        assert(c.key_eq() == test_compare<std::equal_to<int> >());
        assert((c.get_allocator() == test_allocator<std::pair<const int, std::string> >()));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string,
                                   test_hash<std::hash<int> >,
                                   test_compare<std::equal_to<int> >,
                                   min_allocator<std::pair<const int, std::string> >
                                   > C;
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
        C c(input_iterator<P*>(a), input_iterator<P*>(a + sizeof(a)/sizeof(a[0])),
            7,
            test_hash<std::hash<int> >(8)
           );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        typedef std::pair<C::const_iterator, C::const_iterator> Eq;
        Eq eq = c.equal_range(1);
        assert(std::distance(eq.first, eq.second) == 2);
        C::const_iterator i = eq.first;
        assert(i->first == 1);
        assert(i->second == "one");
        ++i;
        assert(i->first == 1);
        assert(i->second == "four");
        eq = c.equal_range(2);
        assert(std::distance(eq.first, eq.second) == 2);
        i = eq.first;
        assert(i->first == 2);
        assert(i->second == "two");
        ++i;
        assert(i->first == 2);
        assert(i->second == "four");

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
        assert(c.max_load_factor() == 1);
        assert(c.hash_function() == test_hash<std::hash<int> >(8));
        assert(c.key_eq() == test_compare<std::equal_to<int> >());
        assert((c.get_allocator() == min_allocator<std::pair<const int, std::string> >()));
    }
#endif
}
