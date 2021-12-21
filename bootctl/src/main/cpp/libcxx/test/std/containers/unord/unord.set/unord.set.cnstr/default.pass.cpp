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
// class unordered_set

// unordered_set();

#include <unordered_set>
#include <cassert>

#include "test_macros.h"
#include "../../../NotConstructible.h"
#include "../../../test_compare.h"
#include "../../../test_hash.h"
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_set<NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   test_allocator<NotConstructible>
                                   > C;
        C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() == (test_allocator<NotConstructible>()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_set<NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   min_allocator<NotConstructible>
                                   > C;
        C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() == (min_allocator<NotConstructible>()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef explicit_allocator<NotConstructible> A;
        typedef std::unordered_set<NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   A
                                   > C;
        {
        C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() == A());
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
        }
        {
        A a;
        C c(a);
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() == a);
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
        }
    }
    {
        std::unordered_set<int> c = {};
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#endif
}
