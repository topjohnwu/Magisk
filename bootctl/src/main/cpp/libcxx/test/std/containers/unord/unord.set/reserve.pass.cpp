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

// void reserve(size_type n);

#include <unordered_set>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class C>
void test(const C& c)
{
    assert(c.size() == 4);
    assert(c.count(1) == 1);
    assert(c.count(2) == 1);
    assert(c.count(3) == 1);
    assert(c.count(4) == 1);
}

void reserve_invariant(size_t n) // LWG #2156
{
    for (size_t i = 0; i < n; ++i)
    {
        std::unordered_set<size_t> c;
        c.reserve(n);
        size_t buckets = c.bucket_count();
        for (size_t j = 0; j < i; ++j)
        {
            c.insert(i);
            assert(buckets == c.bucket_count());
        }
    }
}

int main()
{
    {
        typedef std::unordered_set<int> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        test(c);
        assert(c.bucket_count() >= 5);
        c.reserve(3);
        LIBCPP_ASSERT(c.bucket_count() == 5);
        test(c);
        c.max_load_factor(2);
        c.reserve(3);
        assert(c.bucket_count() >= 2);
        test(c);
        c.reserve(31);
        assert(c.bucket_count() >= 16);
        test(c);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_set<int, std::hash<int>,
                                      std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        test(c);
        assert(c.bucket_count() >= 5);
        c.reserve(3);
        LIBCPP_ASSERT(c.bucket_count() == 5);
        test(c);
        c.max_load_factor(2);
        c.reserve(3);
        assert(c.bucket_count() >= 2);
        test(c);
        c.reserve(31);
        assert(c.bucket_count() >= 16);
        test(c);
    }
#endif
    reserve_invariant(20);
}
