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
//   Iter next(Iter x, Iter::difference_type n = 1);

// LWG #2353 relaxed the requirement on next from ForwardIterator to InputIterator

#include <iterator>
#include <cassert>

#include "test_iterators.h"

template <class It>
void
test(It i, typename std::iterator_traits<It>::difference_type n, It x)
{
    assert(std::next(i, n) == x);

    It (*next)(It, typename std::iterator_traits<It>::difference_type) = std::next;
    assert(next(i, n) == x);
}

template <class It>
void
test(It i, It x)
{
    assert(std::next(i) == x);
}

#if TEST_STD_VER > 14
template <class It>
constexpr bool
constexpr_test(It i, typename std::iterator_traits<It>::difference_type n, It x)
{
    return std::next(i, n) == x;
}

template <class It>
constexpr bool
constexpr_test(It i, It x)
{
    return std::next(i) == x;
}
#endif

int main()
{
    {
    const char* s = "1234567890";
    test(input_iterator<const char*>(s), 10, input_iterator<const char*>(s+10));
    test(forward_iterator<const char*>(s), 10, forward_iterator<const char*>(s+10));
    test(bidirectional_iterator<const char*>(s), 10, bidirectional_iterator<const char*>(s+10));
    test(random_access_iterator<const char*>(s), 10, random_access_iterator<const char*>(s+10));
    test(s, 10, s+10);

    test(input_iterator<const char*>(s), input_iterator<const char*>(s+1));
    test(forward_iterator<const char*>(s), forward_iterator<const char*>(s+1));
    test(bidirectional_iterator<const char*>(s), bidirectional_iterator<const char*>(s+1));
    test(random_access_iterator<const char*>(s), random_access_iterator<const char*>(s+1));
    test(s, s+1);
    }
#if TEST_STD_VER > 14
    {
    constexpr const char* s = "1234567890";
    static_assert( constexpr_test(input_iterator<const char*>(s), 10, input_iterator<const char*>(s+10)), "" );
    static_assert( constexpr_test(forward_iterator<const char*>(s), 10, forward_iterator<const char*>(s+10)), "" );
    static_assert( constexpr_test(bidirectional_iterator<const char*>(s), 10, bidirectional_iterator<const char*>(s+10)), "" );
    static_assert( constexpr_test(random_access_iterator<const char*>(s), 10, random_access_iterator<const char*>(s+10)), "" );
    static_assert( constexpr_test(s, 10, s+10), "" );

    static_assert( constexpr_test(input_iterator<const char*>(s), input_iterator<const char*>(s+1)), "" );
    static_assert( constexpr_test(forward_iterator<const char*>(s), forward_iterator<const char*>(s+1)), "" );
    static_assert( constexpr_test(bidirectional_iterator<const char*>(s), bidirectional_iterator<const char*>(s+1)), "" );
    static_assert( constexpr_test(random_access_iterator<const char*>(s), random_access_iterator<const char*>(s+1)), "" );
    static_assert( constexpr_test(s, s+1), "" );
    }
#endif
}
