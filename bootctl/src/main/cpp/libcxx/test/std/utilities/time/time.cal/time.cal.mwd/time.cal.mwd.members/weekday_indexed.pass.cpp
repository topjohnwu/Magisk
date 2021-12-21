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

// constexpr chrono::weekday_indexed weekday_indexed() const noexcept;
//  Returns: wdi_

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

    ASSERT_NOEXCEPT(                           std::declval<const month_weekday>().weekday_indexed());
    ASSERT_SAME_TYPE(weekday_indexed, decltype(std::declval<const month_weekday>().weekday_indexed()));

    static_assert( month_weekday{month{}, weekday_indexed{}}.weekday_indexed() == weekday_indexed{}, "");

    for (unsigned i = 1; i <= 10; ++i)
    {
        month_weekday md(std::chrono::March, weekday_indexed{Sunday, i});
        assert( static_cast<unsigned>(md.weekday_indexed().weekday() == Sunday));
        assert( static_cast<unsigned>(md.weekday_indexed().index() == i));
    }
}
