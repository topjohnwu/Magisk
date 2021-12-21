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
// class month_day;

// constexpr bool operator==(const month_day& x, const month_day& y) noexcept;
//   Returns: x.month() == y.month() && x.day() == y.day().
//
// constexpr bool operator< (const month_day& x, const month_day& y) noexcept;
//   Returns:
//      If x.month() < y.month() returns true.
//      Otherwise, if x.month() > y.month() returns false.
//      Otherwise, returns x.day() < y.day().

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using day       = std::chrono::day;
    using month     = std::chrono::month;
    using month_day = std::chrono::month_day;

    AssertComparisons6AreNoexcept<month_day>();
    AssertComparisons6ReturnBool<month_day>();

    static_assert( testComparisons6(
        month_day{std::chrono::January, day{1}},
        month_day{std::chrono::January, day{1}},
        true, false), "");

    static_assert( testComparisons6(
        month_day{std::chrono::January, day{1}},
        month_day{std::chrono::January, day{2}},
        false, true), "");

    static_assert( testComparisons6(
        month_day{std::chrono::January,  day{1}},
        month_day{std::chrono::February, day{1}},
        false, true), "");

//  same day, different months
    for (unsigned i = 1; i < 12; ++i)
        for (unsigned j = 1; j < 12; ++j)
            assert((testComparisons6(
                month_day{month{i}, day{1}},
                month_day{month{j}, day{1}},
                i == j, i < j )));

//  same month, different days
    for (unsigned i = 1; i < 31; ++i)
        for (unsigned j = 1; j < 31; ++j)
            assert((testComparisons6(
                month_day{month{2}, day{i}},
                month_day{month{2}, day{j}},
                i == j, i < j )));

}
