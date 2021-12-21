//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <algorithm>

// template<class T, class Compare>
//   T
//   min(initializer_list<T> t, Compare comp);

#include <algorithm>
#include <functional>
#include <cassert>

#include "test_macros.h"

int main()
{
    int i = std::min({2, 3, 1}, std::greater<int>());
    assert(i == 3);
    i = std::min({2, 1, 3}, std::greater<int>());
    assert(i == 3);
    i = std::min({3, 1, 2}, std::greater<int>());
    assert(i == 3);
    i = std::min({3, 2, 1}, std::greater<int>());
    assert(i == 3);
    i = std::min({1, 2, 3}, std::greater<int>());
    assert(i == 3);
    i = std::min({1, 3, 2}, std::greater<int>());
    assert(i == 3);
#if TEST_STD_VER >= 14
    {
    static_assert(std::min({1, 3, 2}, std::greater<int>()) == 3, "");
    static_assert(std::min({2, 1, 3}, std::greater<int>()) == 3, "");
    static_assert(std::min({3, 2, 1}, std::greater<int>()) == 3, "");
    }
#endif
}
