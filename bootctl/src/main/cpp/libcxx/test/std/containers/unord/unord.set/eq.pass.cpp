//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// template <class Key, class Hash, class Pred, class Alloc>
// bool
// operator==(const unordered_set<Key, Hash, Pred, Alloc>& x,
//            const unordered_set<Key, Hash, Pred, Alloc>& y);
//
// template <class Key, class Hash, class Pred, class Alloc>
// bool
// operator!=(const unordered_set<Key, Hash, Pred, Alloc>& x,
//            const unordered_set<Key, Hash, Pred, Alloc>& y);

#include <unordered_set>
#include <cassert>

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
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(!(c1 == c2));
        assert( (c1 != c2));
    }
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
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert( (c1 == c2));
        assert(!(c1 != c2));
    }
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
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        c2.rehash(30);
        assert( (c1 == c2));
        assert(!(c1 != c2));
        c2.insert(P(90));
        assert(!(c1 == c2));
        assert( (c1 != c2));
        c1.insert(P(90));
        assert( (c1 == c2));
        assert(!(c1 != c2));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_set<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> C;
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
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(!(c1 == c2));
        assert( (c1 != c2));
    }
    {
        typedef std::unordered_set<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> C;
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
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert( (c1 == c2));
        assert(!(c1 != c2));
    }
    {
        typedef std::unordered_set<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> C;
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
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        c2.rehash(30);
        assert( (c1 == c2));
        assert(!(c1 != c2));
        c2.insert(P(90));
        assert(!(c1 == c2));
        assert( (c1 != c2));
        c1.insert(P(90));
        assert( (c1 == c2));
        assert(!(c1 != c2));
    }
#endif
}
