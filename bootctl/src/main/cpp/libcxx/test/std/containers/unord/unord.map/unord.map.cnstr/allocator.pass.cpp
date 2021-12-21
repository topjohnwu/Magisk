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

// explicit unordered_map(const allocator_type& __a);

#include <unordered_map>
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
        typedef std::unordered_map<NotConstructible, NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   test_allocator<std::pair<const NotConstructible,
                                                                  NotConstructible> >
                                   > C;
        C c(test_allocator<std::pair<const NotConstructible, NotConstructible> >(10));
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() ==
               (test_allocator<std::pair<const NotConstructible, NotConstructible> >(10)));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<NotConstructible, NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   min_allocator<std::pair<const NotConstructible,
                                                                 NotConstructible> >
                                   > C;
        C c(min_allocator<std::pair<const NotConstructible, NotConstructible> >{});
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() ==
               (min_allocator<std::pair<const NotConstructible, NotConstructible> >()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef explicit_allocator<std::pair<const NotConstructible, NotConstructible>> A;
        typedef std::unordered_map<NotConstructible, NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   A
                                   > C;
        C c(A{});
        LIBCPP_ASSERT(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() == A{});
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#if TEST_STD_VER > 11
    {
        typedef NotConstructible T;
        typedef test_allocator<std::pair<const T, T>> A;
        typedef test_hash<std::hash<T>> HF;
        typedef test_compare<std::equal_to<T>> Comp;
        typedef std::unordered_map<T, T, HF, Comp, A> C;

        A a(10);
        C c(2, a);
        LIBCPP_ASSERT(c.bucket_count() == 2);
        assert(c.hash_function() == HF());
        assert(c.key_eq() == Comp());
        assert(c.get_allocator() == a);
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
    {
        typedef NotConstructible T;
        typedef test_allocator<std::pair<const T, T>> A;
        typedef test_hash<std::hash<T>> HF;
        typedef test_compare<std::equal_to<T>> Comp;
        typedef std::unordered_map<T, T, HF, Comp, A> C;

        A a(10);
        HF hf(12);
        C c(2, hf, a);
        LIBCPP_ASSERT(c.bucket_count() == 2);
        assert(c.hash_function() == hf);
        assert(!(c.hash_function() == HF()));
        assert(c.key_eq() == Comp());
        assert(c.get_allocator() == a);
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#endif
#endif
}
