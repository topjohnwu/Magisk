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
// class year_month_day_last;

// constexpr chrono::day day() const noexcept;
//  Returns: d_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year                = std::chrono::year;
    using month               = std::chrono::month;
    using month_day_last      = std::chrono::month_day_last;
    using year_month_day_last = std::chrono::year_month_day_last;

    ASSERT_NOEXCEPT(                std::declval<const year_month_day_last>().year());
    ASSERT_SAME_TYPE(year, decltype(std::declval<const year_month_day_last>().year()));

    for (int i = 1; i <= 50; ++i)
    {
        year_month_day_last ym(year{i}, month_day_last{month{}});
        assert( static_cast<int>(ym.year()) == i);
    }
}
