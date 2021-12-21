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
// class year_month_weekday_last;

// constexpr chrono::year year() const noexcept;
//  Returns: d_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using year                    = std::chrono::year;
    using month                   = std::chrono::month;
    using weekday                 = std::chrono::weekday;
    using weekday_last            = std::chrono::weekday_last;
    using year_month_weekday_last = std::chrono::year_month_weekday_last;

    ASSERT_NOEXCEPT(                std::declval<const year_month_weekday_last>().year());
    ASSERT_SAME_TYPE(year, decltype(std::declval<const year_month_weekday_last>().year()));

    static_assert( year_month_weekday_last{year{}, month{}, weekday_last{weekday{}}}.year() == year{}, "");

    for (int i = 1; i <= 50; ++i)
    {
        year_month_weekday_last ymwdl(year{i}, month{1}, weekday_last{weekday{}});
        assert(static_cast<int>(ymwdl.year()) == i);
    }
}
