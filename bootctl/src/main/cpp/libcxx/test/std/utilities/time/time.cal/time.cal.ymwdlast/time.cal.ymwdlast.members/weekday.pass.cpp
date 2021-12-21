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

// constexpr chrono::weekday weekday() const noexcept;
//  Returns: wdi_.weekday()

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

    ASSERT_NOEXCEPT(                   std::declval<const year_month_weekday_last>().weekday());
    ASSERT_SAME_TYPE(weekday, decltype(std::declval<const year_month_weekday_last>().weekday()));

    static_assert( year_month_weekday_last{year{}, month{}, weekday_last{weekday{}}}.weekday() == weekday{}, "");

    for (unsigned i = 1; i <= 50; ++i)
    {
        year_month_weekday_last ymwdl(year{1}, month{1}, weekday_last{weekday{i}});
        assert(static_cast<unsigned>(ymwdl.weekday()) == i);
    }
}
