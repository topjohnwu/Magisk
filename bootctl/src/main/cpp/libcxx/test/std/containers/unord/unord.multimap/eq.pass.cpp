//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// template <class Key, class T, class Hash, class Pred, class Alloc>
// bool
// operator==(const unordered_multimap<Key, T, Hash, Pred, Alloc>& x,
//            const unordered_multimap<Key, T, Hash, Pred, Alloc>& y);
//
// template <class Key, class T, class Hash, class Pred, class Alloc>
// bool
// operator!=(const unordered_multimap<Key, T, Hash, Pred, Alloc>& x,
//            const unordered_multimap<Key, T, Hash, Pred, Alloc>& y);

#include <unordered_map>
#include <string>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(!(c1 == c2));
        assert( (c1 != c2));
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert( (c1 == c2));
        assert(!(c1 != c2));
    }
    {
        typedef std::unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        c2.rehash(30);
        assert( (c1 == c2));
        assert(!(c1 != c2));
        c2.insert(P(90, "ninety"));
        assert(!(c1 == c2));
        assert( (c1 != c2));
        c1.insert(P(90, "ninety"));
        assert( (c1 == c2));
        assert(!(c1 != c2));
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c1(std::begin(a), std::end(a));
        const C c2;
        assert(!(c1 == c2));
        assert( (c1 != c2));
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        const C c1(std::begin(a), std::end(a));
        const C c2 = c1;
        assert( (c1 == c2));
        assert(!(c1 != c2));
    }
    {
        typedef std::unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(10, "ten"),
            P(20, "twenty"),
            P(20, "twenty 2"),
            P(30, "thirty"),
            P(40, "forty"),
            P(50, "fifty"),
            P(50, "fifty 2"),
            P(50, "fifty 3"),
            P(60, "sixty"),
            P(70, "seventy"),
            P(80, "eighty"),
        };
        C c1(std::begin(a), std::end(a));
        C c2 = c1;
        c2.rehash(30);
        assert( (c1 == c2));
        assert(!(c1 != c2));
        c2.insert(P(90, "ninety"));
        assert(!(c1 == c2));
        assert( (c1 != c2));
        c1.insert(P(90, "ninety"));
        assert( (c1 == c2));
        assert(!(c1 != c2));
    }
#endif
}
