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

// explicit unordered_multiset(const allocator_type& __a);

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
        typedef std::unordered_multiset<NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   test_allocator<NotConstructible>
                                   > C;
        C c(test_allocator<NotConstructible>(10));
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() == test_allocator<NotConstructible>(10));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   min_allocator<NotConstructible>
                                   > C;
        C c(min_allocator<NotConstructible>{});
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() == min_allocator<NotConstructible>());
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER > 11
    {
        typedef NotConstructible T;
        typedef test_hash<std::hash<T>> HF;
        typedef test_compare<std::equal_to<T>> Comp;
        typedef test_allocator<T> A;
        typedef std::unordered_multiset<T, HF, Comp, A> C;

        A a(43);
        C c(3, a);
        LIBCPP_ASSERT(c.bucket_count() == 3);
        assert(c.hash_function() == HF());
        assert(c.key_eq() == Comp ());
        assert(c.get_allocator() == a);
        assert(!(c.get_allocator() == A()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef NotConstructible T;
        typedef test_hash<std::hash<T>> HF;
        typedef test_compare<std::equal_to<T>> Comp;
        typedef test_allocator<T> A;
        typedef std::unordered_multiset<T, HF, Comp, A> C;

        HF hf(42);
        A a(43);
        C c(4, hf, a);
        LIBCPP_ASSERT(c.bucket_count() == 4);
        assert(c.hash_function() == hf);
        assert(!(c.hash_function() == HF()));
        assert(c.key_eq() == Comp ());
        assert(c.get_allocator() == a);
        assert(!(c.get_allocator() == A()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#endif
#endif
}
