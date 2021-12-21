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

// void swap(unordered_set& u);

#include <unordered_set>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "../../test_compare.h"
#include "../../test_hash.h"
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef test_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        C c1(0, Hash(1), Compare(1), Alloc(1, 1));
        C c2(0, Hash(2), Compare(2), Alloc(1, 2));
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator().get_id() == 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator().get_id() == 2);
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef test_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        typedef int P;
        P a2[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        C c1(0, Hash(1), Compare(1), Alloc(1, 1));
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc(1, 2));
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(*c1.find(10) == 10);
        assert(*c1.find(20) == 20);
        assert(*c1.find(30) == 30);
        assert(*c1.find(40) == 40);
        assert(*c1.find(50) == 50);
        assert(*c1.find(60) == 60);
        assert(*c1.find(70) == 70);
        assert(*c1.find(80) == 80);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator().get_id() == 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator().get_id() == 2);
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef test_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        typedef int P;
        P a1[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc(1, 1));
        C c2(0, Hash(2), Compare(2), Alloc(1, 2));
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator().get_id() == 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.count(1) == 1);
        assert(c2.count(2) == 1);
        assert(c2.count(3) == 1);
        assert(c2.count(4) == 1);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator().get_id() == 2);
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef test_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        typedef int P;
        P a1[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        P a2[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc(1, 1));
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc(1, 2));
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(*c1.find(10) == 10);
        assert(*c1.find(20) == 20);
        assert(*c1.find(30) == 30);
        assert(*c1.find(40) == 40);
        assert(*c1.find(50) == 50);
        assert(*c1.find(60) == 60);
        assert(*c1.find(70) == 70);
        assert(*c1.find(80) == 80);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator().get_id() == 1);
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.count(1) == 1);
        assert(c2.count(2) == 1);
        assert(c2.count(3) == 1);
        assert(c2.count(4) == 1);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator().get_id() == 2);
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }

    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef other_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        C c1(0, Hash(1), Compare(1), Alloc(1));
        C c2(0, Hash(2), Compare(2), Alloc(2));
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc(2));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc(1));
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef other_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        typedef int P;
        P a2[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        C c1(0, Hash(1), Compare(1), Alloc(1));
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc(2));
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(*c1.find(10) == 10);
        assert(*c1.find(20) == 20);
        assert(*c1.find(30) == 30);
        assert(*c1.find(40) == 40);
        assert(*c1.find(50) == 50);
        assert(*c1.find(60) == 60);
        assert(*c1.find(70) == 70);
        assert(*c1.find(80) == 80);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc(2));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc(1));
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef other_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        typedef int P;
        P a1[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc(1));
        C c2(0, Hash(2), Compare(2), Alloc(2));
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc(2));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.count(1) == 1);
        assert(c2.count(2) == 1);
        assert(c2.count(3) == 1);
        assert(c2.count(4) == 1);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc(1));
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef other_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        typedef int P;
        P a1[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        P a2[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc(1));
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc(2));
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(*c1.find(10) == 10);
        assert(*c1.find(20) == 20);
        assert(*c1.find(30) == 30);
        assert(*c1.find(40) == 40);
        assert(*c1.find(50) == 50);
        assert(*c1.find(60) == 60);
        assert(*c1.find(70) == 70);
        assert(*c1.find(80) == 80);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc(2));
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.count(1) == 1);
        assert(c2.count(2) == 1);
        assert(c2.count(3) == 1);
        assert(c2.count(4) == 1);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc(1));
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef min_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        C c1(0, Hash(1), Compare(1), Alloc());
        C c2(0, Hash(2), Compare(2), Alloc());
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef min_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        typedef int P;
        P a2[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        C c1(0, Hash(1), Compare(1), Alloc());
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc());
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(*c1.find(10) == 10);
        assert(*c1.find(20) == 20);
        assert(*c1.find(30) == 30);
        assert(*c1.find(40) == 40);
        assert(*c1.find(50) == 50);
        assert(*c1.find(60) == 60);
        assert(*c1.find(70) == 70);
        assert(*c1.find(80) == 80);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        LIBCPP_ASSERT(c2.bucket_count() == 0);
        assert(c2.size() == 0);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef min_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        typedef int P;
        P a1[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc());
        C c2(0, Hash(2), Compare(2), Alloc());
        c2.max_load_factor(2);
        c1.swap(c2);

        LIBCPP_ASSERT(c1.bucket_count() == 0);
        assert(c1.size() == 0);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.count(1) == 1);
        assert(c2.count(2) == 1);
        assert(c2.count(3) == 1);
        assert(c2.count(4) == 1);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
    {
        typedef test_hash<std::hash<int> > Hash;
        typedef test_compare<std::equal_to<int> > Compare;
        typedef min_allocator<int> Alloc;
        typedef std::unordered_set<int, Hash, Compare, Alloc> C;
        typedef int P;
        P a1[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        P a2[] =
        {
            P(10),
            P(20),
            P(30),
            P(40),
            P(50),
            P(60),
            P(70),
            P(80)
        };
        C c1(std::begin(a1), std::end(a1), 0, Hash(1), Compare(1), Alloc());
        C c2(std::begin(a2), std::end(a2), 0, Hash(2), Compare(2), Alloc());
        c2.max_load_factor(2);
        c1.swap(c2);

        assert(c1.bucket_count() >= 8);
        assert(c1.size() == 8);
        assert(*c1.find(10) == 10);
        assert(*c1.find(20) == 20);
        assert(*c1.find(30) == 30);
        assert(*c1.find(40) == 40);
        assert(*c1.find(50) == 50);
        assert(*c1.find(60) == 60);
        assert(*c1.find(70) == 70);
        assert(*c1.find(80) == 80);
        assert(c1.hash_function() == Hash(2));
        assert(c1.key_eq() == Compare(2));
        assert(c1.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c1.begin(), c1.end())) == c1.size());
        assert(static_cast<std::size_t>(std::distance(c1.cbegin(), c1.cend())) == c1.size());
        assert(c1.max_load_factor() == 2);

        assert(c2.bucket_count() >= 4);
        assert(c2.size() == 4);
        assert(c2.count(1) == 1);
        assert(c2.count(2) == 1);
        assert(c2.count(3) == 1);
        assert(c2.count(4) == 1);
        assert(c2.hash_function() == Hash(1));
        assert(c2.key_eq() == Compare(1));
        assert(c2.get_allocator() == Alloc());
        assert(static_cast<std::size_t>(std::distance(c2.begin(), c2.end())) == c2.size());
        assert(static_cast<std::size_t>(std::distance(c2.cbegin(), c2.cend())) == c2.size());
        assert(c2.max_load_factor() == 1);
    }
#endif
}
