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

// local_iterator       begin (size_type n);
// local_iterator       end   (size_type n);
// const_local_iterator begin (size_type n) const;
// const_local_iterator end   (size_type n) const;
// const_local_iterator cbegin(size_type n) const;
// const_local_iterator cend  (size_type n) const;

#include <unordered_map>
#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        typedef C::local_iterator I;
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
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        assert(i->first == 1);
        assert(i->second == "one");
        ++i;
        assert(i->first == 1);
        assert(i->second == "four");
        i->first = 2;

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        assert(i->first == 2);
        assert(i->second == "two");
        ++i;
        assert(i->first == 2);
        assert(i->second == "four");

        b = c.bucket(3);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        typedef C::const_local_iterator I;
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
        C::size_type b = c.bucket(0);
        I i = c.begin(b);
        I j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        assert(i->first == 1);
        assert(i->second == "one");
        ++i;
        assert(i->first == 1);
        assert(i->second == "four");

        b = c.bucket(2);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 2);
        assert(i->first == 2);
        assert(i->second == "two");
        ++i;
        assert(i->first == 2);
        assert(i->second == "four");

        b = c.bucket(3);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.begin(b);
        j = c.end(b);
        assert(std::distance(i, j) == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        typedef C::const_local_iterator I;
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
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        assert(i->first == 1);
        assert(i->second == "one");
        ++i;
        assert(i->first == 1);
        assert(i->second == "four");

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        assert(i->first == 2);
        assert(i->second == "two");
        ++i;
        assert(i->first == 2);
        assert(i->second == "four");

        b = c.bucket(3);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        typedef C::const_local_iterator I;
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
        C::size_type b = c.bucket(0);
        I i = c.cbegin(b);
        I j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(1);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        assert(i->first == 1);
        assert(i->second == "one");
        ++i;
        assert(i->first == 1);
        assert(i->second == "four");

        b = c.bucket(2);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 2);
        assert(i->first == 2);
        assert(i->second == "two");
        ++i;
        assert(i->first == 2);
        assert(i->second == "four");

        b = c.bucket(3);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 3);
        assert(i->second == "three");

        b = c.bucket(4);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 1);
        assert(i->first == 4);
        assert(i->second == "four");

        b = c.bucket(5);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);

        b = c.bucket(6);
        i = c.cbegin(b);
        j = c.cend(b);
        assert(std::distance(i, j) == 0);
    }
}
