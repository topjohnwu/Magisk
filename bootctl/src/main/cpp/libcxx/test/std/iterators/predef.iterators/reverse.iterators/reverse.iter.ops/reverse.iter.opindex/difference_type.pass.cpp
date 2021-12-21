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
//   unspecified operator[](difference_type n) const;

#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
void
test(It i, typename std::iterator_traits<It>::difference_type n,
     typename std::iterator_traits<It>::value_type x)
{
    typedef typename std::iterator_traits<It>::value_type value_type;
    const std::reverse_iterator<It> r(i);
    value_type rr = r[n];
    assert(rr == x);
}

int main()
{
    const char* s = "1234567890";
    test(random_access_iterator<const char*>(s+5), 4, '1');
    test(s+5, 4, '1');

#if TEST_STD_VER > 14
    {
    constexpr const char *p = "123456789";
    typedef std::reverse_iterator<const char *> RI;
    constexpr RI it1 = std::make_reverse_iterator(p + 5);
    static_assert(it1[0] == '5', "");
    static_assert(it1[4] == '1', "");
    }
#endif
}
