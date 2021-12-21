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

// constexpr year operator+(const year& x, const years& y) noexcept;
//   Returns: year(int{x} + y.count()).
//
// constexpr year operator+(const years& x, const year& y) noexcept;
//   Returns: y + x


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename Y, typename Ys>
constexpr bool testConstexpr()
{
    Y y{1001};
    Ys offset{23};
    if (y + offset != Y{1024}) return false;
    if (offset + y != Y{1024}) return false;
    return true;
}

int main()
{
    using year  = std::chrono::year;
    using years = std::chrono::years;

    ASSERT_NOEXCEPT(                std::declval<year>() + std::declval<years>());
    ASSERT_SAME_TYPE(year, decltype(std::declval<year>() + std::declval<years>()));

    ASSERT_NOEXCEPT(                std::declval<years>() + std::declval<year>());
    ASSERT_SAME_TYPE(year, decltype(std::declval<years>() + std::declval<year>()));

    static_assert(testConstexpr<year, years>(), "");

    year y{1223};
    for (int i = 1100; i <= 1110; ++i)
    {
        year y1 = y + years{i};
        year y2 = years{i} + y;
        assert(y1 == y2);
        assert(static_cast<int>(y1) == i + 1223);
        assert(static_cast<int>(y2) == i + 1223);
    }
}
