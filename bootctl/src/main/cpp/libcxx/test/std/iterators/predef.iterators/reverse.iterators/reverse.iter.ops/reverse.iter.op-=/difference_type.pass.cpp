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

// requires RandomAccessIterator<Iter>
//   constexpr reverse_iterator& operator-=(difference_type n);
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
    std::reverse_iterator<It> r(i);
    std::reverse_iterator<It>& rr = r -= n;
    assert(r.base() == x);
    assert(&rr == &r);
}

int main()
{
    const char* s = "1234567890";
    test(random_access_iterator<const char*>(s+5), 5, random_access_iterator<const char*>(s+10));
    test(s+5, 5, s+10);

#if TEST_STD_VER > 14
    {
        constexpr const char *p = "123456789";
        constexpr auto it1 = std::make_reverse_iterator(p+5);
        constexpr auto it2 = std::make_reverse_iterator(p) -= 5;
        static_assert(it1 == it2, "");
    }
#endif
}
