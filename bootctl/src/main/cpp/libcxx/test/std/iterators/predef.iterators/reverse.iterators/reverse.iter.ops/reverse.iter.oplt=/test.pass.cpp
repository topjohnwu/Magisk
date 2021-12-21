//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// reverse_iterator

// template <RandomAccessIterator Iter1, RandomAccessIterator Iter2>
//   requires HasGreater<Iter1, Iter2>
//   constexpr bool
//   operator<=(const reverse_iterator<Iter1>& x, const reverse_iterator<Iter2>& y);
//
//   constexpr in C++17

#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
void
test(It l, It r, bool x)
{
    const std::reverse_iterator<It> r1(l);
    const std::reverse_iterator<It> r2(r);
    assert((r1 <= r2) == x);
}

int main()
{
    const char* s = "1234567890";
    test(random_access_iterator<const char*>(s), random_access_iterator<const char*>(s), true);
    test(random_access_iterator<const char*>(s), random_access_iterator<const char*>(s+1), false);
    test(random_access_iterator<const char*>(s+1), random_access_iterator<const char*>(s), true);
    test(s, s, true);
    test(s, s+1, false);
    test(s+1, s, true);

#if TEST_STD_VER > 14
    {
        constexpr const char *p = "123456789";
        typedef std::reverse_iterator<const char *> RI;
        constexpr RI it1 = std::make_reverse_iterator(p);
        constexpr RI it2 = std::make_reverse_iterator(p);
        constexpr RI it3 = std::make_reverse_iterator(p+1);
        static_assert( (it1 <= it2), "");
        static_assert(!(it1 <= it3), "");
    }
#endif
}
