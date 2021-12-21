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

// constexpr bool operator==(const month_weekday_last& x, const month_weekday_last& y) noexcept;
//   Returns: x.month() == y.month()
//
// constexpr bool operator< (const month_weekday_last& x, const month_weekday_last& y) noexcept;
//   Returns: x.month() < y.month()


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using month              = std::chrono::month;
    using weekday_last       = std::chrono::weekday_last;
    using weekday            = std::chrono::weekday;
    using month_weekday_last = std::chrono::month_weekday_last;

    constexpr month January = std::chrono::January;
    constexpr weekday Tuesday = std::chrono::Tuesday;
    constexpr weekday Wednesday = std::chrono::Wednesday;

    AssertComparisons2AreNoexcept<month_weekday_last>();
    AssertComparisons2ReturnBool<month_weekday_last>();

    static_assert( testComparisons2(
        month_weekday_last{std::chrono::January, weekday_last{Tuesday}},
        month_weekday_last{std::chrono::January, weekday_last{Tuesday}},
        true), "");

    static_assert( testComparisons2(
        month_weekday_last{std::chrono::January, weekday_last{Tuesday}},
        month_weekday_last{std::chrono::January, weekday_last{Wednesday}},
        false), "");

//  vary the months
    for (unsigned i = 1; i < 12; ++i)
        for (unsigned j = 1; j < 12; ++j)
            assert((testComparisons2(
                month_weekday_last{month{i}, weekday_last{Tuesday}},
                month_weekday_last{month{j}, weekday_last{Tuesday}},
            i == j)));

//  vary the weekday
    for (unsigned i = 0; i < 6; ++i)
        for (unsigned j = 0; j < 6; ++j)
            assert((testComparisons2(
                month_weekday_last{January, weekday_last{weekday{i}}},
                month_weekday_last{January, weekday_last{weekday{j}}},
            i == j)));

//  both different
        assert((testComparisons2(
            month_weekday_last{month{1}, weekday_last{weekday{1}}},
            month_weekday_last{month{2}, weekday_last{weekday{2}}},
        false)));
}
