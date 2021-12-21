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

// iterator find(const key_type& k);

#include <unordered_set>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_multiset<int> C;
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
        C c(std::begin(a), std::end(a));
        C::iterator i = c.find(30);
        assert(*i == 30);
        i = c.find(5);
        assert(i == c.cend());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multiset<int, std::hash<int>,
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
        C c(std::begin(a), std::end(a));
        C::iterator i = c.find(30);
        assert(*i == 30);
        i = c.find(5);
        assert(i == c.cend());
    }
#endif
}
