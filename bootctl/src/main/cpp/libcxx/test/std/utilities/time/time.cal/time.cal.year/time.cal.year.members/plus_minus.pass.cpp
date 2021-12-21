//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <chrono>
// class year;

// constexpr year operator+() const noexcept;
// constexpr year operator-() const noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename Y>
constexpr bool testConstexpr()
{
    Y y1{1};
    if (static_cast<int>(+y1) !=  1) return false;
    if (static_cast<int>(-y1) != -1) return false;
    return true;
}

int main()
{
    using year  = std::chrono::year;

    ASSERT_NOEXCEPT(+std::declval<year>());
    ASSERT_NOEXCEPT(-std::declval<year>());

    ASSERT_SAME_TYPE(year, decltype(+std::declval<year>()));
    ASSERT_SAME_TYPE(year, decltype(-std::declval<year>()));

    static_assert(testConstexpr<year>(), "");

    for (int i = 10000; i <= 10020; ++i)
    {
        year year(i);
        assert(static_cast<int>(+year) ==  i);
        assert(static_cast<int>(-year) == -i);
    }
}
