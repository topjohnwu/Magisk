//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// move_iterator

// template <InputIterator Iter>
//   move_iterator<Iter>
//   make_move_iterator(const Iter& i);
//
//  constexpr in C++17

#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
void
test(It i)
{
    const std::move_iterator<It> r(i);
    assert(std::make_move_iterator(i) == r);
}

int main()
{
    {
    char s[] = "1234567890";
    test(input_iterator<char*>(s+5));
    test(forward_iterator<char*>(s+5));
    test(bidirectional_iterator<char*>(s+5));
    test(random_access_iterator<char*>(s+5));
    test(s+5);
    }
    {
    int a[] = {1,2,3,4};
    TEST_IGNORE_NODISCARD std::make_move_iterator(a+4);
    TEST_IGNORE_NODISCARD std::make_move_iterator(a); // test for LWG issue 2061
    }

#if TEST_STD_VER > 14
    {
    constexpr const char *p = "123456789";
    constexpr auto iter = std::make_move_iterator<const char *>(p);
    static_assert(iter.base() == p);
    }
#endif
}
