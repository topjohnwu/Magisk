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
// class year_month_day;

// constexpr bool operator==(const year_month_day& x, const year_month_day& y) noexcept;
//   Returns: x.year() == y.year() && x.month() == y.month().
//
// constexpr bool operator< (const year_month_day& x, const year_month_day& y) noexcept;
//   Returns:
//      If x.year() < y.year() returns true.
//      Otherwise, if x.year() > y.year() returns false.
//      Otherwise, if x.month() < y.month() returns true.
//      Otherwise, if x.month() > y.month() returns false.
//      Otherwise, returns x.day() < y.day()


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using day        = std::chrono::day;
    using year       = std::chrono::year;
    using month      = std::chrono::month;
    using year_month_day = std::chrono::year_month_day;

    AssertComparisons6AreNoexcept<year_month_day>();
    AssertComparisons6ReturnBool<year_month_day>();

    constexpr month January = std::chrono::January;
    constexpr month February = std::chrono::February;

    static_assert( testComparisons6(
        year_month_day{year{1234}, January, day{1}},
        year_month_day{year{1234}, January, day{1}},
        true, false), "");

//  different day
    static_assert( testComparisons6(
        year_month_day{year{1234}, January, day{1}},
        year_month_day{year{1234}, January, day{2}},
        false, true), "");

//  different month
    static_assert( testComparisons6(
        year_month_day{year{1234}, January, day{1}},
        year_month_day{year{1234}, February, day{1}},
        false, true), "");

//  different year
    static_assert( testComparisons6(
        year_month_day{year{1234}, January, day{1}},
        year_month_day{year{1235}, January, day{1}},
        false, true), "");


//  different month and day
    static_assert( testComparisons6(
        year_month_day{year{1234}, January, day{2}},
        year_month_day{year{1234}, February, day{1}},
        false, true), "");

//  different year and month
    static_assert( testComparisons6(
        year_month_day{year{1234}, February, day{1}},
        year_month_day{year{1235}, January, day{1}},
        false, true), "");

//  different year and day
    static_assert( testComparisons6(
        year_month_day{year{1234}, January, day{2}},
        year_month_day{year{1235}, January, day{1}},
        false, true), "");

//  different year, month and day
    static_assert( testComparisons6(
        year_month_day{year{1234}, February, day{2}},
        year_month_day{year{1235}, January, day{1}},
        false, true), "");


//  same year, different days
    for (unsigned i = 1; i < 28; ++i)
        for (unsigned j = 1; j < 28; ++j)
            assert((testComparisons6(
                year_month_day{year{1234}, January, day{i}},
                year_month_day{year{1234}, January, day{j}},
                i == j, i < j )));

//  same year, different months
    for (unsigned i = 1; i < 12; ++i)
        for (unsigned j = 1; j < 12; ++j)
            assert((testComparisons6(
                year_month_day{year{1234}, month{i}, day{12}},
                year_month_day{year{1234}, month{j}, day{12}},
                i == j, i < j )));

//  same month, different years
    for (int i = 1000; i < 20; ++i)
        for (int j = 1000; j < 20; ++j)
        assert((testComparisons6(
            year_month_day{year{i}, January, day{12}},
            year_month_day{year{j}, January, day{12}},
            i == j, i < j )));
}
