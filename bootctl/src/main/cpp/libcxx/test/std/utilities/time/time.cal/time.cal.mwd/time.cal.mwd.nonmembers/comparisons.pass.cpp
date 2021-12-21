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
// class month_weekday;

// constexpr bool operator==(const month_weekday& x, const month_weekday& y) noexcept;
//   Returns: x.month() == y.month() && x.day() == y.day().
//

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using month_weekday   = std::chrono::month_weekday;
    using month           = std::chrono::month;
    using weekday_indexed = std::chrono::weekday_indexed;
    using weekday         = std::chrono::weekday;

    constexpr weekday Sunday = std::chrono::Sunday;
    constexpr weekday Monday = std::chrono::Monday;

    AssertComparisons2AreNoexcept<month_weekday>();
    AssertComparisons2ReturnBool<month_weekday>();

    static_assert( testComparisons2(
        month_weekday{std::chrono::January, weekday_indexed{Sunday, 1}},
        month_weekday{std::chrono::January, weekday_indexed{Sunday, 1}},
        true), "");

    static_assert( testComparisons2(
        month_weekday{std::chrono::January, weekday_indexed{Sunday, 1}},
        month_weekday{std::chrono::January, weekday_indexed{Sunday, 2}},
        false), "");

    static_assert( testComparisons2(
        month_weekday{std::chrono::January,  weekday_indexed{Sunday, 1}},
        month_weekday{std::chrono::February, weekday_indexed{Sunday, 1}},
        false), "");

    static_assert( testComparisons2(
        month_weekday{std::chrono::January, weekday_indexed{Monday, 1}},
        month_weekday{std::chrono::January, weekday_indexed{Sunday, 2}},
        false), "");

    static_assert( testComparisons2(
        month_weekday{std::chrono::January,  weekday_indexed{Monday, 1}},
        month_weekday{std::chrono::February, weekday_indexed{Sunday, 1}},
        false), "");

//  same day, different months
    for (unsigned i = 1; i < 12; ++i)
        for (unsigned j = 1; j < 12; ++j)
            assert((testComparisons2(
                month_weekday{month{i}, weekday_indexed{Sunday, 1}},
                month_weekday{month{j}, weekday_indexed{Sunday, 1}},
                i == j)));

//  same month, different weeks
    for (unsigned i = 1; i < 5; ++i)
        for (unsigned j = 1; j < 5; ++j)
            assert((testComparisons2(
                month_weekday{month{2}, weekday_indexed{Sunday, i}},
                month_weekday{month{2}, weekday_indexed{Sunday, j}},
                i == j)));

//  same month, different days
    for (unsigned i = 0; i < 6; ++i)
        for (unsigned j = 0; j < 6; ++j)
            assert((testComparisons2(
                month_weekday{month{2}, weekday_indexed{weekday{i}, 2}},
                month_weekday{month{2}, weekday_indexed{weekday{j}, 2}},
                i == j)));
}
