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

// iterator       begin()        {return __table_.begin();}
// iterator       end()          {return __table_.end();}
// const_iterator begin()  const {return __table_.begin();}
// const_iterator end()    const {return __table_.end();}
// const_iterator cbegin() const {return __table_.begin();}
// const_iterator cend()   const {return __table_.end();}

#include <unordered_map>
#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(std::distance(c.begin(), c.end()) == c.size());
        assert(std::distance(c.cbegin(), c.cend()) == c.size());
        C::iterator i = c.begin();
        i->second = "ONE";
        assert(i->second == "ONE");
        i->first = 2;
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        const C c(a, a + sizeof(a)/sizeof(a[0]));
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.size() == 6);
        assert(std::distance(c.begin(), c.end()) == c.size());
        assert(std::distance(c.cbegin(), c.cend()) == c.size());
    }
}
