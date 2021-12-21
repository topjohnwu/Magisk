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
// class weekday;

// constexpr bool operator==(const weekday& x, const weekday& y) noexcept;
// constexpr bool operator!=(const weekday& x, const weekday& y) noexcept;


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using weekday = std::chrono::weekday;

    AssertComparisons2AreNoexcept<weekday>();
    AssertComparisons2ReturnBool<weekday>();

    static_assert(testComparisons2Values<weekday>(0U ,0U), "");
    static_assert(testComparisons2Values<weekday>(0U, 1U), "");

//  Some 'ok' values as well
    static_assert(testComparisons2Values<weekday>(5U, 5U), "");
    static_assert(testComparisons2Values<weekday>(5U, 2U), "");

    for (unsigned i = 0; i < 6; ++i)
        for (unsigned j = 0; j < 6; ++j)
            assert(testComparisons2Values<weekday>(i, j));
}
