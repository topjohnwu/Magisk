//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

//   All of these became constexpr in C++17
//
// template <InputIterator Iter>
//   constexpr void advance(Iter& i, Iter::difference_type n);
//
// template <BidirectionalIterator Iter>
//   constexpr void advance(Iter& i, Iter::difference_type n);
//
// template <RandomAccessIterator Iter>
//   constexpr void advance(Iter& i, Iter::difference_type n);

#include <iterator>
#include <cassert>

#include "test_iterators.h"

template <class It>
void
test(It i, typename std::iterator_traits<It>::difference_type n, It x)
{
    std::advance(i, n);
    assert(i == x);
}

#if TEST_STD_VER > 14
template <class It>
constexpr bool
constepxr_test(It i, typename std::iterator_traits<It>::difference_type n, It x)
{
    std::advance(i, n);
    return i == x;
}
#endif

int main()
{
    {
    const char* s = "1234567890";
    test(input_iterator<const char*>(s), 10, input_iterator<const char*>(s+10));
    test(forward_iterator<const char*>(s), 10, forward_iterator<const char*>(s+10));
    test(bidirectional_iterator<const char*>(s+5), 5, bidirectional_iterator<const char*>(s+10));
    test(bidirectional_iterator<const char*>(s+5), -5, bidirectional_iterator<const char*>(s));
    test(random_access_iterator<const char*>(s+5), 5, random_access_iterator<const char*>(s+10));
    test(random_access_iterator<const char*>(s+5), -5, random_access_iterator<const char*>(s));
    test(s+5, 5, s+10);
    test(s+5, -5, s);
    }
#if TEST_STD_VER > 14
    {
    constexpr const char* s = "1234567890";
    static_assert( constepxr_test(input_iterator<const char*>(s), 10, input_iterator<const char*>(s+10)), "" );
    static_assert( constepxr_test(forward_iterator<const char*>(s), 10, forward_iterator<const char*>(s+10)), "" );
    static_assert( constepxr_test(bidirectional_iterator<const char*>(s+5), 5, bidirectional_iterator<const char*>(s+10)), "" );
    static_assert( constepxr_test(bidirectional_iterator<const char*>(s+5), -5, bidirectional_iterator<const char*>(s)), "" );
    static_assert( constepxr_test(random_access_iterator<const char*>(s+5), 5, random_access_iterator<const char*>(s+10)), "" );
    static_assert( constepxr_test(random_access_iterator<const char*>(s+5), -5, random_access_iterator<const char*>(s)), "" );
    static_assert( constepxr_test(s+5, 5, s+10), "" );
    static_assert( constepxr_test(s+5, -5, s), "" );
    }
#endif
}
