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

// size_type bucket_size(size_type n) const

#ifdef _LIBCPP_DEBUG
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

#include <unordered_set>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

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
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 5);
        LIBCPP_ASSERT(c.bucket_size(0) == 0);
        LIBCPP_ASSERT(c.bucket_size(1) == 1);
        LIBCPP_ASSERT(c.bucket_size(2) == 1);
        LIBCPP_ASSERT(c.bucket_size(3) == 1);
        LIBCPP_ASSERT(c.bucket_size(4) == 1);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_set<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> C;
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
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 5);
        LIBCPP_ASSERT(c.bucket_size(0) == 0);
        LIBCPP_ASSERT(c.bucket_size(1) == 1);
        LIBCPP_ASSERT(c.bucket_size(2) == 1);
        LIBCPP_ASSERT(c.bucket_size(3) == 1);
        LIBCPP_ASSERT(c.bucket_size(4) == 1);
    }
#endif
#if _LIBCPP_DEBUG_LEVEL >= 1
    {
        typedef std::unordered_set<int> C;
        C c;
        C::size_type i = c.bucket_size(3);
        assert(false);
    }
#endif
}
