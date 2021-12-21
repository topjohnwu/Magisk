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
// class weekday_indexed;

// constexpr bool ok() const noexcept;
//  Returns: wd_.ok() && 1 <= index_ && index_ <= 5

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using weekday         = std::chrono::weekday;
    using weekday_indexed = std::chrono::weekday_indexed;

    ASSERT_NOEXCEPT(                std::declval<const weekday_indexed>().ok());
    ASSERT_SAME_TYPE(bool, decltype(std::declval<const weekday_indexed>().ok()));

    static_assert(!weekday_indexed{}.ok(),                       "");
    static_assert( weekday_indexed{std::chrono::Sunday, 2}.ok(), "");

    assert(!(weekday_indexed(std::chrono::Tuesday, 0).ok()));
    for (unsigned i = 1; i <= 5; ++i)
    {
        weekday_indexed wdi(std::chrono::Tuesday, i);
        assert( wdi.ok());
    }

    for (unsigned i = 6; i <= 20; ++i)
    {
        weekday_indexed wdi(std::chrono::Tuesday, i);
        assert(!wdi.ok());
    }

//  Not a valid weekday
    assert(!(weekday_indexed(weekday{9U}, 1).ok()));
}
