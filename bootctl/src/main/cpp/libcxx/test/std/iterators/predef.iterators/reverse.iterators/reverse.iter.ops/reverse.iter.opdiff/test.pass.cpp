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
//   requires HasMinus<Iter2, Iter1>
//   constexpr auto operator-(const reverse_iterator<Iter1>& x, const reverse_iterator<Iter2>& y)
//   -> decltype(y.base() - x.base());
//
// constexpr in C++17

#include <iterator>
#include <cstddef>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It1, class It2>
void
test(It1 l, It2 r, std::ptrdiff_t x)
{
    const std::reverse_iterator<It1> r1(l);
    const std::reverse_iterator<It2> r2(r);
    assert((r1 - r2) == x);
}

int main()
{
    char s[3] = {0};
    test(random_access_iterator<const char*>(s), random_access_iterator<char*>(s), 0);
    test(random_access_iterator<char*>(s), random_access_iterator<const char*>(s+1), 1);
    test(random_access_iterator<const char*>(s+1), random_access_iterator<char*>(s), -1);
    test(s, s, 0);
    test(s, s+1, 1);
    test(s+1, s, -1);

#if TEST_STD_VER > 14
    {
        constexpr const char *p = "123456789";
        typedef std::reverse_iterator<const char *> RI;
        constexpr RI it1 = std::make_reverse_iterator(p);
        constexpr RI it2 = std::make_reverse_iterator(p+1);
        static_assert( it1 - it2 ==  1, "");
        static_assert( it2 - it1 == -1, "");
    }
#endif
}
