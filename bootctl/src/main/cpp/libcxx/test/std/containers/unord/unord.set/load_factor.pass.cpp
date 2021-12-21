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

// float load_factor() const

#include <unordered_set>
#include <cassert>
#include <cfloat>
#include <cmath>

#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_set<int> C;
        typedef int P;
        P a[] =
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
        const C c(std::begin(a), std::end(a));
        assert(std::fabs(c.load_factor() - (float)c.size()/c.bucket_count()) < FLT_EPSILON);
    }
    {
        typedef std::unordered_set<int> C;
        const C c;
        assert(c.load_factor() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_set<int, std::hash<int>,
                                      std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
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
        const C c(std::begin(a), std::end(a));
        assert(std::fabs(c.load_factor() - (float)c.size()/c.bucket_count()) < FLT_EPSILON);
    }
    {
        typedef std::unordered_set<int, std::hash<int>,
                                      std::equal_to<int>, min_allocator<int>> C;
        const C c;
        assert(c.load_factor() == 0);
    }
#endif
}
