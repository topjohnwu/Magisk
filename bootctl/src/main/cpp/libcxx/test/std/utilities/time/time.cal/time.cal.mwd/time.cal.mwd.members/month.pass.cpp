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
// class month_weekday;

// constexpr chrono::month month() const noexcept;
//  Returns: wd_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using month_weekday   = std::chrono::month_weekday;
    using month           = std::chrono::month;
    using weekday         = std::chrono::weekday;
    using weekday_indexed = std::chrono::weekday_indexed;

    constexpr weekday Sunday = std::chrono::Sunday;

    ASSERT_NOEXCEPT(                 std::declval<const month_weekday>().month());
    ASSERT_SAME_TYPE(month, decltype(std::declval<const month_weekday>().month()));

    static_assert( month_weekday{}.month() == month{}, "");

    for (unsigned i = 1; i <= 50; ++i)
    {
        month_weekday md(month{i}, weekday_indexed{Sunday, 1});
        assert( static_cast<unsigned>(md.month()) == i);
    }
}
