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
// class year_month_weekday;

// constexpr bool operator==(const year_month_weekday& x, const year_month_weekday& y) noexcept;
//   Returns: x.year() == y.year() && x.month() == y.month() && x.weekday_indexed() == y.weekday_indexed()
//


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using year               = std::chrono::year;
    using month              = std::chrono::month;
    using weekday_indexed    = std::chrono::weekday_indexed;
    using weekday            = std::chrono::weekday;
    using year_month_weekday = std::chrono::year_month_weekday;

    AssertComparisons2AreNoexcept<year_month_weekday>();
    AssertComparisons2ReturnBool<year_month_weekday>();

    constexpr month January   = std::chrono::January;
    constexpr month February  = std::chrono::February;
    constexpr weekday Tuesday = std::chrono::Tuesday;

    static_assert( testComparisons2(
        year_month_weekday{year{1234}, January, weekday_indexed{Tuesday, 1}},
        year_month_weekday{year{1234}, January, weekday_indexed{Tuesday, 1}},
        true), "");

//  different day
    static_assert( testComparisons2(
        year_month_weekday{year{1234}, January, weekday_indexed{Tuesday, 1}},
        year_month_weekday{year{1234}, January, weekday_indexed{Tuesday, 2}},
        false), "");

//  different month
    static_assert( testComparisons2(
        year_month_weekday{year{1234}, January,  weekday_indexed{Tuesday, 1}},
        year_month_weekday{year{1234}, February, weekday_indexed{Tuesday, 1}},
        false), "");

//  different year
    static_assert( testComparisons2(
        year_month_weekday{year{1234}, January, weekday_indexed{Tuesday, 1}},
        year_month_weekday{year{1235}, January, weekday_indexed{Tuesday, 1}},
        false), "");


//  different month and day
    static_assert( testComparisons2(
        year_month_weekday{year{1234}, January,  weekday_indexed{Tuesday, 1}},
        year_month_weekday{year{1234}, February, weekday_indexed{Tuesday, 2}},
        false), "");

//  different year and month
    static_assert( testComparisons2(
        year_month_weekday{year{1234}, February, weekday_indexed{Tuesday, 1}},
        year_month_weekday{year{1235}, January,  weekday_indexed{Tuesday, 1}},
        false), "");

//  different year and day
    static_assert( testComparisons2(
        year_month_weekday{year{1234}, January, weekday_indexed{Tuesday, 2}},
        year_month_weekday{year{1235}, January, weekday_indexed{Tuesday, 1}},
        false), "");

//  different year, month and day
    static_assert( testComparisons2(
        year_month_weekday{year{1234}, February, weekday_indexed{Tuesday, 2}},
        year_month_weekday{year{1235}, January,  weekday_indexed{Tuesday, 1}},
        false), "");


//  same year, different days
    for (unsigned i = 1; i < 28; ++i)
        for (unsigned j = 1; j < 28; ++j)
            assert((testComparisons2(
                year_month_weekday{year{1234}, January, weekday_indexed{Tuesday, i}},
                year_month_weekday{year{1234}, January, weekday_indexed{Tuesday, j}},
                i == j)));

//  same year, different months
    for (unsigned i = 1; i < 12; ++i)
        for (unsigned j = 1; j < 12; ++j)
            assert((testComparisons2(
                year_month_weekday{year{1234}, month{i}, weekday_indexed{Tuesday, 1}},
                year_month_weekday{year{1234}, month{j}, weekday_indexed{Tuesday, 1}},
                i == j)));

//  same month, different years
    for (int i = 1000; i < 20; ++i)
        for (int j = 1000; j < 20; ++j)
        assert((testComparisons2(
            year_month_weekday{year{i}, January, weekday_indexed{Tuesday, 1}},
            year_month_weekday{year{j}, January, weekday_indexed{Tuesday, 1}},
            i == j)));
}
