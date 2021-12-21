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

// size_type bucket_count() const;

#include <unordered_map>
#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        const C c;
        LIBCPP_ASSERT(c.bucket_count() == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 8);
    }
}
