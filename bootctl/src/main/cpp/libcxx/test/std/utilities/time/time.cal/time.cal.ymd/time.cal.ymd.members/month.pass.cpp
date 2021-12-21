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
// class year_month_day;

// constexpr chrono::month month() const noexcept;
//  Returns: wd_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year           = std::chrono::year;
    using month          = std::chrono::month;
    using day            = std::chrono::day;
    using year_month_day = std::chrono::year_month_day;

    ASSERT_NOEXCEPT(                 std::declval<const year_month_day>().month());
    ASSERT_SAME_TYPE(month, decltype(std::declval<const year_month_day>().month()));

    static_assert( year_month_day{}.month() == month{}, "");

    for (unsigned i = 1; i <= 50; ++i)
    {
        year_month_day ymd(year{1234}, month{i}, day{12});
        assert( static_cast<unsigned>(ymd.month()) == i);
    }
}
