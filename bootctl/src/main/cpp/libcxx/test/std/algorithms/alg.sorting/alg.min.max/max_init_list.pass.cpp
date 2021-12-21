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

// template <class T>
//   T
//   max(initializer_list<T> t);

#include <algorithm>
#include <cassert>

#include "test_macros.h"

int main()
{
    int i = std::max({2, 3, 1});
    assert(i == 3);
    i = std::max({2, 1, 3});
    assert(i == 3);
    i = std::max({3, 1, 2});
    assert(i == 3);
    i = std::max({3, 2, 1});
    assert(i == 3);
    i = std::max({1, 2, 3});
    assert(i == 3);
    i = std::max({1, 3, 2});
    assert(i == 3);
#if TEST_STD_VER >= 14
    {
    static_assert(std::max({1, 3, 2}) == 3, "");
    static_assert(std::max({2, 1, 3}) == 3, "");
    static_assert(std::max({3, 2, 1}) == 3, "");
    }
#endif
}
