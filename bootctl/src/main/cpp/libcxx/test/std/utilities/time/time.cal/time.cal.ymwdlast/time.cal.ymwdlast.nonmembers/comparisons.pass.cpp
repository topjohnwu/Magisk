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

// constexpr bool operator==(const year_month_weekday_last& x, const year_month_weekday_last& y) noexcept;
//   Returns: x.year() == y.year() && x.month() == y.month() && x.weekday_last() == y.weekday_last()
//


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using year                    = std::chrono::year;
    using month                   = std::chrono::month;
    using weekday                 = std::chrono::weekday;
    using weekday_last            = std::chrono::weekday_last;
    using year_month_weekday_last = std::chrono::year_month_weekday_last;

    AssertComparisons2AreNoexcept<year_month_weekday_last>();
    AssertComparisons2ReturnBool<year_month_weekday_last>();

    constexpr month January   = std::chrono::January;
    constexpr month February  = std::chrono::February;
    constexpr weekday Tuesday = std::chrono::Tuesday;
    constexpr weekday Wednesday = std::chrono::Wednesday;

    static_assert( testComparisons2(
        year_month_weekday_last{year{1234}, January, weekday_last{Tuesday}},
        year_month_weekday_last{year{1234}, January, weekday_last{Tuesday}},
        true), "");

//  different day
    static_assert( testComparisons2(
        year_month_weekday_last{year{1234}, January, weekday_last{Tuesday}},
        year_month_weekday_last{year{1234}, January, weekday_last{Wednesday}},
        false), "");

//  different month
    static_assert( testComparisons2(
        year_month_weekday_last{year{1234}, January,  weekday_last{Tuesday}},
        year_month_weekday_last{year{1234}, February, weekday_last{Tuesday}},
        false), "");

//  different year
    static_assert( testComparisons2(
        year_month_weekday_last{year{1234}, January, weekday_last{Tuesday}},
        year_month_weekday_last{year{1235}, January, weekday_last{Tuesday}},
        false), "");


//  different month and day
    static_assert( testComparisons2(
        year_month_weekday_last{year{1234}, January,  weekday_last{Tuesday}},
        year_month_weekday_last{year{1234}, February, weekday_last{Wednesday}},
        false), "");

//  different year and month
    static_assert( testComparisons2(
        year_month_weekday_last{year{1234}, February, weekday_last{Tuesday}},
        year_month_weekday_last{year{1235}, January,  weekday_last{Tuesday}},
        false), "");

//  different year and day
    static_assert( testComparisons2(
        year_month_weekday_last{year{1234}, January, weekday_last{Wednesday}},
        year_month_weekday_last{year{1235}, January, weekday_last{Tuesday}},
        false), "");

//  different year, month and day
    static_assert( testComparisons2(
        year_month_weekday_last{year{1234}, February, weekday_last{Wednesday}},
        year_month_weekday_last{year{1235}, January,  weekday_last{Tuesday}},
        false), "");


//  same year, different days
    for (unsigned i = 1; i < 28; ++i)
        for (unsigned j = 1; j < 28; ++j)
            assert((testComparisons2(
                year_month_weekday_last{year{1234}, January, weekday_last{weekday{i}}},
                year_month_weekday_last{year{1234}, January, weekday_last{weekday{j}}},
                i == j)));

//  same year, different months
    for (unsigned i = 1; i < 12; ++i)
        for (unsigned j = 1; j < 12; ++j)
            assert((testComparisons2(
                year_month_weekday_last{year{1234}, month{i}, weekday_last{Tuesday}},
                year_month_weekday_last{year{1234}, month{j}, weekday_last{Tuesday}},
                i == j)));

//  same month, different years
    for (int i = 1000; i < 20; ++i)
        for (int j = 1000; j < 20; ++j)
        assert((testComparisons2(
            year_month_weekday_last{year{i}, January, weekday_last{Tuesday}},
            year_month_weekday_last{year{j}, January, weekday_last{Tuesday}},
            i == j)));
}
