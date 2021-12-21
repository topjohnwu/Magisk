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

//                     year() = default;
//  explicit constexpr year(int m) noexcept;
//  explicit constexpr operator int() const noexcept;

//  Effects: Constructs an object of type year by initializing y_ with y.
//    The value held is unspecified if d is not in the range [0, 255].

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year = std::chrono::year;

    ASSERT_NOEXCEPT(year{});
    ASSERT_NOEXCEPT(year(0U));
    ASSERT_NOEXCEPT(static_cast<int>(year(0U)));

    constexpr year y0{};
    static_assert(static_cast<int>(y0) == 0, "");

    constexpr year y1{1};
    static_assert(static_cast<int>(y1) == 1, "");

    for (int i = 0; i <= 2550; i += 7)
    {
        year year(i);
        assert(static_cast<int>(year) == i);
    }
}
