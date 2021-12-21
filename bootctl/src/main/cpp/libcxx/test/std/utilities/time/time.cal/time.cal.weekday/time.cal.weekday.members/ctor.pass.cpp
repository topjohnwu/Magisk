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
// class weekday;

//                     weekday() = default;
//  explicit constexpr weekday(unsigned wd) noexcept;
//  constexpr weekday(const sys_days& dp) noexcept;
//  explicit constexpr weekday(const local_days& dp) noexcept;
//
//  explicit constexpr operator unsigned() const noexcept;

//  Effects: Constructs an object of type weekday by initializing m_ with m.
//    The value held is unspecified if d is not in the range [0, 255].

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using weekday = std::chrono::weekday;

    ASSERT_NOEXCEPT(weekday{});
    ASSERT_NOEXCEPT(weekday(1));
    ASSERT_NOEXCEPT(static_cast<unsigned>(weekday(1)));

    constexpr weekday m0{};
    static_assert(static_cast<unsigned>(m0) == 0, "");

    constexpr weekday m1{1};
    static_assert(static_cast<unsigned>(m1) == 1, "");

    for (unsigned i = 0; i <= 255; ++i)
    {
        weekday m(i);
        assert(static_cast<unsigned>(m) == i);
    }

// TODO - sys_days and local_days ctor tests
}
