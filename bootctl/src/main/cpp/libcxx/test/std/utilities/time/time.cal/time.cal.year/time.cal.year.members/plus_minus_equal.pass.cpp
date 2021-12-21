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

// constexpr year& operator+=(const years& d) noexcept;
// constexpr year& operator-=(const years& d) noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename Y, typename Ys>
constexpr bool testConstexpr()
{
    Y y1{1};
    if (static_cast<int>(y1 += Ys{ 1}) !=  2) return false;
    if (static_cast<int>(y1 += Ys{ 2}) !=  4) return false;
    if (static_cast<int>(y1 += Ys{ 8}) != 12) return false;
    if (static_cast<int>(y1 -= Ys{ 1}) != 11) return false;
    if (static_cast<int>(y1 -= Ys{ 2}) !=  9) return false;
    if (static_cast<int>(y1 -= Ys{ 8}) !=  1) return false;
    return true;
}

int main()
{
    using year  = std::chrono::year;
    using years = std::chrono::years;

    ASSERT_NOEXCEPT(std::declval<year&>() += std::declval<years>());
    ASSERT_NOEXCEPT(std::declval<year&>() -= std::declval<years>());

    ASSERT_SAME_TYPE(year&, decltype(std::declval<year&>() += std::declval<years>()));
    ASSERT_SAME_TYPE(year&, decltype(std::declval<year&>() -= std::declval<years>()));

    static_assert(testConstexpr<year, years>(), "");

    for (int i = 10000; i <= 10020; ++i)
    {
        year year(i);
        assert(static_cast<int>(year += years{10}) == i + 10);
        assert(static_cast<int>(year)              == i + 10);
        assert(static_cast<int>(year -= years{ 9}) == i +  1);
        assert(static_cast<int>(year)              == i +  1);
    }
}
