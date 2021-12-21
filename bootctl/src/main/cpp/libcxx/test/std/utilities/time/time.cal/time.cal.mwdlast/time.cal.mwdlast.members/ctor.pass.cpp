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
// class month_weekday_last;

//  constexpr month_weekday_last(const chrono::month& m,
//                               const chrono::weekday_last& wdl) noexcept;
//
//  Effects:  Constructs an object of type month_weekday_last by
//            initializing m_ with m, and wdl_ with wdl.
//
//     constexpr chrono::month        month() const noexcept;
//     constexpr chrono::weekday_last weekday_last()  const noexcept;
//     constexpr bool                 ok()    const noexcept;


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using month              = std::chrono::month;
    using weekday            = std::chrono::weekday;
    using weekday_last       = std::chrono::weekday_last;
    using month_weekday_last = std::chrono::month_weekday_last;

    constexpr month January = std::chrono::January;
    constexpr weekday Tuesday = std::chrono::Tuesday;

    ASSERT_NOEXCEPT(month_weekday_last{January, weekday_last{Tuesday}});

//  bad month
    constexpr month_weekday_last mwdl1{month{}, weekday_last{Tuesday}};
    static_assert( mwdl1.month() == month{},                      "");
    static_assert( mwdl1.weekday_last() == weekday_last{Tuesday}, "");
    static_assert(!mwdl1.ok(),                                    "");

//  bad weekday_last
    constexpr month_weekday_last mwdl2{January, weekday_last{weekday{16}}};
    static_assert( mwdl2.month() == January,                          "");
    static_assert( mwdl2.weekday_last() == weekday_last{weekday{16}}, "");
    static_assert(!mwdl2.ok(),                                        "");

//  Good month and weekday_last
    constexpr month_weekday_last mwdl3{January, weekday_last{weekday{4}}};
    static_assert( mwdl3.month() == January,                         "");
    static_assert( mwdl3.weekday_last() == weekday_last{weekday{4}}, "");
    static_assert( mwdl3.ok(),                                       "");
}
