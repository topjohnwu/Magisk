//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <iterator>

// reverse_iterator

// template <class Iterator>
//   constexpr reverse_iterator<Iterator>
//     make_reverse_iterator(Iterator i);
//
//   constexpr in C++17

#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
void
test(It i)
{
    const std::reverse_iterator<It> r = std::make_reverse_iterator(i);
    assert(r.base() == i);
}

int main()
{
    const char* s = "1234567890";
    random_access_iterator<const char*>b(s);
    random_access_iterator<const char*>e(s+10);
    while ( b != e )
        test ( b++ );

#if TEST_STD_VER > 14
    {
        constexpr const char *p = "123456789";
        constexpr auto it1 = std::make_reverse_iterator(p);
        static_assert(it1.base() == p, "");
    }
#endif
}

