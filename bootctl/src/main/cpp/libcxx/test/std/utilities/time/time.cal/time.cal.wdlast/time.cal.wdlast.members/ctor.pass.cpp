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
// class weekday_last;

//  explicit constexpr weekday_last(const chrono::weekday& wd) noexcept;
//
//  Effects: Constructs an object of type weekday_last by initializing wd_ with wd.
//
//  constexpr chrono::weekday weekday() const noexcept;
//  constexpr bool ok() const noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using weekday      = std::chrono::weekday;
    using weekday_last = std::chrono::weekday_last;

    ASSERT_NOEXCEPT(weekday_last{weekday{}});

    constexpr weekday_last wdl0{weekday{}};
    static_assert( wdl0.weekday() == weekday{}, "");
    static_assert( wdl0.ok(),                   "");

    constexpr weekday_last wdl1 {weekday{1}};
    static_assert( wdl1.weekday() == weekday{1}, "");
    static_assert( wdl1.ok(),                    "");

    for (unsigned i = 0; i <= 255; ++i)
    {
        weekday_last wdl{weekday{i}};
        assert(wdl.weekday() == weekday{i});
    }
}
