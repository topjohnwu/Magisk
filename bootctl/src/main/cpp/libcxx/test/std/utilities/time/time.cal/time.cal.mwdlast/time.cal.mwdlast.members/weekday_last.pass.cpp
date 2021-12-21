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

// constexpr chrono::weekday_last weekday_last() const noexcept;
//  Returns: wdl_

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

    ASSERT_NOEXCEPT(                        std::declval<const month_weekday_last>().weekday_last());
    ASSERT_SAME_TYPE(weekday_last, decltype(std::declval<const month_weekday_last>().weekday_last()));

    static_assert( month_weekday_last{month{}, lastTuesday}.weekday_last() == lastTuesday, "");

    for (unsigned i = 1; i <= 50; ++i)
    {
        month_weekday_last mdl(January, weekday_last{weekday{i}});
        assert( static_cast<unsigned>(mdl.weekday_last().weekday()) == i);
    }
}
