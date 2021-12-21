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

// pointer operator->() const;
//
//  constexpr in C++17

#include <iterator>
#include <cassert>

#include "test_macros.h"

template <class It>
void
test(It i)
{
    std::move_iterator<It> r(i);
    assert(r.operator->() == i);
}

int main()
{
    char s[] = "123";
    test(s);

#if TEST_STD_VER > 14
    {
    constexpr const char *p = "123456789";
    typedef std::move_iterator<const char *> MI;
    constexpr MI it1 = std::make_move_iterator(p);
    constexpr MI it2 = std::make_move_iterator(p+1);
    static_assert(it1.operator->() == p, "");
    static_assert(it2.operator->() == p + 1, "");
    }
#endif
}
