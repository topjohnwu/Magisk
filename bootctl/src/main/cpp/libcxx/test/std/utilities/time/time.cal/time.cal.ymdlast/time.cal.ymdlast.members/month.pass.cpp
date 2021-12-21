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

// constexpr chrono::month month() const noexcept;
//  Returns: wd_

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

    ASSERT_NOEXCEPT(                 std::declval<const year_month_day_last>().month());
    ASSERT_SAME_TYPE(month, decltype(std::declval<const year_month_day_last>().month()));

    for (unsigned i = 1; i <= 50; ++i)
    {
        year_month_day_last ymd(year{1234}, month_day_last{month{i}});
        assert( static_cast<unsigned>(ymd.month()) == i);
    }
}
