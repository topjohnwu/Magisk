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

// template <RandomAccessIterator Iterator>
//   constexpr reverse_iterator<Iter>
//   operator+(Iter::difference_type n, const reverse_iterator<Iter>& x);
//
// constexpr in C++17

#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
void
test(It i, typename std::iterator_traits<It>::difference_type n, It x)
{
    const std::reverse_iterator<It> r(i);
    std::reverse_iterator<It> rr = n + r;
    assert(rr.base() == x);
}

int main()
{
    const char* s = "1234567890";
    test(random_access_iterator<const char*>(s+5), 5, random_access_iterator<const char*>(s));
    test(s+5, 5, s);

#if TEST_STD_VER > 14
    {
        constexpr const char *p = "123456789";
        typedef std::reverse_iterator<const char *> RI;
        constexpr RI it1 = std::make_reverse_iterator(p);
        constexpr RI it2 = std::make_reverse_iterator(p + 5);
        constexpr RI it3 = 5 + it2;
        static_assert(it1 != it2, "");
        static_assert(it1 == it3, "");
        static_assert(it2 != it3, "");
    }
#endif
}
