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
// class year_month;

// constexpr chrono::day day() const noexcept;
//  Returns: d_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year       = std::chrono::year;
    using month      = std::chrono::month;
    using year_month = std::chrono::year_month;

    ASSERT_NOEXCEPT(                std::declval<const year_month>().year());
    ASSERT_SAME_TYPE(year, decltype(std::declval<const year_month>().year()));

    static_assert( year_month{}.year() == year{}, "");

    for (int i = 1; i <= 50; ++i)
    {
        year_month ym(year{i}, month{});
        assert( static_cast<int>(ym.year()) == i);
    }
}
