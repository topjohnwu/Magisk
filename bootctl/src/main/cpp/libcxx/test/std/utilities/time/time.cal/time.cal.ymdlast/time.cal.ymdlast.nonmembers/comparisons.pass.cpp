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

// constexpr bool operator==(const year_month_day_last& x, const year_month_day_last& y) noexcept;
//   Returns: x.year() == y.year() && x.month_day_last() == y.month_day_last().
//
// constexpr bool operator< (const year_month_day_last& x, const year_month_day_last& y) noexcept;
//   Returns:
//      If x.year() < y.year(), returns true.
//      Otherwise, if x.year() > y.year(), returns false.
//      Otherwise, returns x.month_day_last() < y.month_day_last()

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using year                = std::chrono::year;
    using month               = std::chrono::month;
    using month_day_last      = std::chrono::month_day_last;
    using year_month_day_last = std::chrono::year_month_day_last;

    AssertComparisons6AreNoexcept<year_month_day_last>();
    AssertComparisons6ReturnBool<year_month_day_last>();

    constexpr month January = std::chrono::January;
    constexpr month February = std::chrono::February;

    static_assert( testComparisons6(
        year_month_day_last{year{1234}, month_day_last{January}},
        year_month_day_last{year{1234}, month_day_last{January}},
        true, false), "");

//  different month
    static_assert( testComparisons6(
        year_month_day_last{year{1234}, month_day_last{January}},
        year_month_day_last{year{1234}, month_day_last{February}},
        false, true), "");

//  different year
    static_assert( testComparisons6(
        year_month_day_last{year{1234}, month_day_last{January}},
        year_month_day_last{year{1235}, month_day_last{January}},
        false, true), "");

//  different month
    static_assert( testComparisons6(
        year_month_day_last{year{1234}, month_day_last{January}},
        year_month_day_last{year{1234}, month_day_last{February}},
        false, true), "");

//  different year and month
    static_assert( testComparisons6(
        year_month_day_last{year{1234}, month_day_last{February}},
        year_month_day_last{year{1235}, month_day_last{January}},
        false, true), "");

//  same year, different months
    for (unsigned i = 1; i < 12; ++i)
        for (unsigned j = 1; j < 12; ++j)
            assert((testComparisons6(
                year_month_day_last{year{1234}, month_day_last{month{i}}},
                year_month_day_last{year{1234}, month_day_last{month{j}}},
                i == j, i < j )));

//  same month, different years
    for (int i = 1000; i < 20; ++i)
        for (int j = 1000; j < 20; ++j)
        assert((testComparisons6(
            year_month_day_last{year{i}, month_day_last{January}},
            year_month_day_last{year{j}, month_day_last{January}},
            i == j, i < j )));
}
