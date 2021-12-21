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
// class year_month;

// constexpr bool operator==(const year_month& x, const year_month& y) noexcept;
//   Returns: x.year() == y.year() && x.month() == y.month().
//
// constexpr bool operator< (const year_month& x, const year_month& y) noexcept;
//   Returns:
//      If x.year() < y.year() returns true.
//      Otherwise, if x.year() > y.year() returns false.
//      Otherwise, returns x.month() < y.month().

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using year       = std::chrono::year;
    using month      = std::chrono::month;
    using year_month = std::chrono::year_month;

    AssertComparisons6AreNoexcept<year_month>();
    AssertComparisons6ReturnBool<year_month>();

    static_assert( testComparisons6(
        year_month{year{1234}, std::chrono::January},
        year_month{year{1234}, std::chrono::January},
        true, false), "");

    static_assert( testComparisons6(
        year_month{year{1234}, std::chrono::January},
        year_month{year{1234}, std::chrono::February},
        false, true), "");

    static_assert( testComparisons6(
        year_month{year{1234}, std::chrono::January},
        year_month{year{1235}, std::chrono::January},
        false, true), "");

//  same year, different months
    for (unsigned i = 1; i < 12; ++i)
        for (unsigned j = 1; j < 12; ++j)
            assert((testComparisons6(
                year_month{year{1234}, month{i}},
                year_month{year{1234}, month{j}},
                i == j, i < j )));

//  same month, different years
    for (int i = 1000; i < 20; ++i)
        for (int j = 1000; j < 20; ++j)
        assert((testComparisons6(
            year_month{year{i}, std::chrono::January},
            year_month{year{j}, std::chrono::January},
            i == j, i < j )));
}
