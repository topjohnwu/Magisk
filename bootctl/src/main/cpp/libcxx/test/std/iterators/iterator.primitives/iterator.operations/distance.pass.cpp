//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// template <InputIterator Iter>
//   Iter::difference_type
//   distance(Iter first, Iter last);
//
// template <RandomAccessIterator Iter>
//   Iter::difference_type
//   distance(Iter first, Iter last);

#include <iterator>
#include <cassert>

#include "test_iterators.h"

template <class It>
void
test(It first, It last, typename std::iterator_traits<It>::difference_type x)
{
    assert(std::distance(first, last) == x);
}

#if TEST_STD_VER > 14
template <class It>
constexpr bool
constexpr_test(It first, It last, typename std::iterator_traits<It>::difference_type x)
{
    return std::distance(first, last) == x;
}
#endif

int main()
{
    {
    const char* s = "1234567890";
    test(input_iterator<const char*>(s), input_iterator<const char*>(s+10), 10);
    test(forward_iterator<const char*>(s), forward_iterator<const char*>(s+10), 10);
    test(bidirectional_iterator<const char*>(s), bidirectional_iterator<const char*>(s+10), 10);
    test(random_access_iterator<const char*>(s), random_access_iterator<const char*>(s+10), 10);
    test(s, s+10, 10);
    }
#if TEST_STD_VER > 14
    {
    constexpr const char* s = "1234567890";
    static_assert( constexpr_test(input_iterator<const char*>(s), input_iterator<const char*>(s+10), 10), "");
    static_assert( constexpr_test(forward_iterator<const char*>(s), forward_iterator<const char*>(s+10), 10), "");
    static_assert( constexpr_test(bidirectional_iterator<const char*>(s), bidirectional_iterator<const char*>(s+10), 10), "");
    static_assert( constexpr_test(random_access_iterator<const char*>(s), random_access_iterator<const char*>(s+10), 10), "");
    static_assert( constexpr_test(s, s+10, 10), "");
    }
#endif
}
