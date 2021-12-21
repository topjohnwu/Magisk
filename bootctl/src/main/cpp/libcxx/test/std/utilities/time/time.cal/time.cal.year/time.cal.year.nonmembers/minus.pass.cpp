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

// constexpr year operator-(const year& x, const years& y) noexcept;
//   Returns: x + -y.
//
// constexpr years operator-(const year& x, const year& y) noexcept;
//   Returns: If x.ok() == true and y.ok() == true, returns a value m in the range
//   [years{0}, years{11}] satisfying y + m == x.
//   Otherwise the value returned is unspecified.
//   [Example: January - February == years{11}. —end example]

extern "C" int printf(const char *, ...);

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename Y, typename Ys>
constexpr bool testConstexpr()
{
    Y y{2313};
    Ys offset{1006};
    if (y - offset != Y{1307}) return false;
    if (y - Y{1307} != offset) return false;
    return true;
}

int main()
{
    using year  = std::chrono::year;
    using years = std::chrono::years;

    ASSERT_NOEXCEPT(                 std::declval<year>() - std::declval<years>());
    ASSERT_SAME_TYPE(year , decltype(std::declval<year>() - std::declval<years>()));

    ASSERT_NOEXCEPT(                 std::declval<year>() - std::declval<year>());
    ASSERT_SAME_TYPE(years, decltype(std::declval<year>() - std::declval<year>()));

    static_assert(testConstexpr<year, years>(), "");

    year y{1223};
    for (int i = 1100; i <= 1110; ++i)
    {
        year  y1 = y - years{i};
        years ys1 = y - year{i};
        assert(static_cast<int>(y1) == 1223 - i);
        assert(ys1.count()          == 1223 - i);
    }
}
