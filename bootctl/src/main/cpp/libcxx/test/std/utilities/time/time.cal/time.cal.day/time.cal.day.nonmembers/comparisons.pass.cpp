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
// class day;

// constexpr bool operator==(const day& x, const day& y) noexcept;
//   Returns: unsigned{x} == unsigned{y}.
// constexpr bool operator<(const day& x, const day& y) noexcept;
//   Returns: unsigned{x} < unsigned{y}.


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using day = std::chrono::day;

    AssertComparisons6AreNoexcept<day>();
    AssertComparisons6ReturnBool<day>();

    static_assert(testComparisons6Values<day>(0U, 0U), "");
    static_assert(testComparisons6Values<day>(0U, 1U), "");

//  Some 'ok' values as well
    static_assert(testComparisons6Values<day>( 5U,  5U), "");
    static_assert(testComparisons6Values<day>( 5U, 10U), "");

    for (unsigned i = 1; i < 10; ++i)
        for (unsigned j = 1; j < 10; ++j)
            assert(testComparisons6Values<day>(i, j));
}
