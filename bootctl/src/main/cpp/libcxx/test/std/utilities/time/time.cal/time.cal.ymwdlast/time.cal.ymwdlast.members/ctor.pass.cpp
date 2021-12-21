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

//  constexpr year_month_weekday_last(const chrono::year& y, const chrono::month& m,
//                               const chrono::weekday_last& wdl) noexcept;
//
//  Effects:  Constructs an object of type year_month_weekday_last by initializing
//                y_ with y, m_ with m, and wdl_ with wdl.
//
//  constexpr chrono::year                 year() const noexcept;
//  constexpr chrono::month               month() const noexcept;
//  constexpr chrono::weekday           weekday() const noexcept;
//  constexpr chrono::weekday_last weekday_last() const noexcept;
//  constexpr bool                           ok() const noexcept;

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

    constexpr month January = std::chrono::January;
    constexpr weekday Tuesday = std::chrono::Tuesday;

    ASSERT_NOEXCEPT(year_month_weekday_last{year{1}, month{1}, weekday_last{Tuesday}});

    constexpr year_month_weekday_last ym1{year{2019}, January, weekday_last{Tuesday}};
    static_assert( ym1.year()         == year{2019},            "");
    static_assert( ym1.month()        == January,               "");
    static_assert( ym1.weekday()      == Tuesday,               "");
    static_assert( ym1.weekday_last() == weekday_last{Tuesday}, "");
    static_assert( ym1.ok(),                                    "");

}
