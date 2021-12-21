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

// iterator       begin()        {return __table_.begin();}
// iterator       end()          {return __table_.end();}
// const_iterator begin()  const {return __table_.begin();}
// const_iterator end()    const {return __table_.end();}
// const_iterator cbegin() const {return __table_.begin();}
// const_iterator cend()   const {return __table_.end();}

#include <unordered_set>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        typedef std::unordered_multiset<int> C;
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
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(std::distance(c.begin(), c.end()) == c.size());
        assert(std::distance(c.cbegin(), c.cend()) == c.size());
        C::iterator i = c.begin();
        assert(*i == 1);
        *i = 2;
    }
    {
        typedef std::unordered_multiset<int> C;
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
        const C c(a, a + sizeof(a)/sizeof(a[0]));
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(std::distance(c.begin(), c.end()) == c.size());
        assert(std::distance(c.cbegin(), c.cend()) == c.size());
    }
}
