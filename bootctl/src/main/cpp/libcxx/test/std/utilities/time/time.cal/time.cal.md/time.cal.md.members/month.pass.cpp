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
// class month_day;

// constexpr chrono::month month() const noexcept;
//  Returns: wd_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using day       = std::chrono::day;
    using month     = std::chrono::month;
    using month_day = std::chrono::month_day;

    ASSERT_NOEXCEPT(                 std::declval<const month_day>().month());
    ASSERT_SAME_TYPE(month, decltype(std::declval<const month_day>().month()));

    static_assert( month_day{}.month() == month{}, "");

    for (unsigned i = 1; i <= 50; ++i)
    {
        month_day md(month{i}, day{1});
        assert( static_cast<unsigned>(md.month()) == i);
    }
}
