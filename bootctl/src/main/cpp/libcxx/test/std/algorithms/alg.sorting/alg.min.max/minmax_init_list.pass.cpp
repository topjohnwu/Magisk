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

// template<class T>
//   pair<T, T>
//   minmax(initializer_list<T> t);

#include <algorithm>
#include <cassert>

#include "test_macros.h"

int main()
{
    assert((std::minmax({1, 2, 3}) == std::pair<int, int>(1, 3)));
    assert((std::minmax({1, 3, 2}) == std::pair<int, int>(1, 3)));
    assert((std::minmax({2, 1, 3}) == std::pair<int, int>(1, 3)));
    assert((std::minmax({2, 3, 1}) == std::pair<int, int>(1, 3)));
    assert((std::minmax({3, 1, 2}) == std::pair<int, int>(1, 3)));
    assert((std::minmax({3, 2, 1}) == std::pair<int, int>(1, 3)));
#if TEST_STD_VER >= 14
    {
    static_assert((std::minmax({1, 2, 3}) == std::pair<int, int>(1, 3)), "");
    static_assert((std::minmax({1, 3, 2}) == std::pair<int, int>(1, 3)), "");
    static_assert((std::minmax({2, 1, 3}) == std::pair<int, int>(1, 3)), "");
    static_assert((std::minmax({2, 3, 1}) == std::pair<int, int>(1, 3)), "");
    static_assert((std::minmax({3, 1, 2}) == std::pair<int, int>(1, 3)), "");
    static_assert((std::minmax({3, 2, 1}) == std::pair<int, int>(1, 3)), "");
    }
#endif
}
