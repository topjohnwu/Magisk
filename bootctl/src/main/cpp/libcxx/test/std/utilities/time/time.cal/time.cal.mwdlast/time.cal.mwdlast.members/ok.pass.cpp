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

// constexpr bool ok() const noexcept;
//  Returns: m_.ok() && wdl_.ok().

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

    constexpr month January            = std::chrono::January;
    constexpr weekday Tuesday          = std::chrono::Tuesday;
    constexpr weekday_last lastTuesday = weekday_last{Tuesday};

    ASSERT_NOEXCEPT(                std::declval<const month_weekday_last>().ok());
    ASSERT_SAME_TYPE(bool, decltype(std::declval<const month_weekday_last>().ok()));

    static_assert(!month_weekday_last{month{}, lastTuesday}.ok(),               ""); // Bad month
    static_assert(!month_weekday_last{January, weekday_last{weekday{12}}}.ok(), ""); // Bad month
    static_assert( month_weekday_last{January, lastTuesday}.ok(),               ""); // Both OK

    for (unsigned i = 0; i <= 50; ++i)
    {
        month_weekday_last mwdl{month{i}, lastTuesday};
        assert( mwdl.ok() == month{i}.ok());
    }

    for (unsigned i = 0; i <= 50; ++i)
    {
        month_weekday_last mwdl{January, weekday_last{weekday{i}}};
        assert( mwdl.ok() == weekday_last{weekday{i}}.ok());
    }
}
