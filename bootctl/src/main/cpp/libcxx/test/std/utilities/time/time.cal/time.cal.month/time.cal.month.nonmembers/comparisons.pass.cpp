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
// class month;

// constexpr bool operator==(const month& x, const month& y) noexcept;
// constexpr bool operator!=(const month& x, const month& y) noexcept;
// constexpr bool operator< (const month& x, const month& y) noexcept;
// constexpr bool operator> (const month& x, const month& y) noexcept;
// constexpr bool operator<=(const month& x, const month& y) noexcept;
// constexpr bool operator>=(const month& x, const month& y) noexcept;


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"


int main()
{
    using month = std::chrono::month;

    AssertComparisons6AreNoexcept<month>();
    AssertComparisons6ReturnBool<month>();

    static_assert(testComparisons6Values<month>(0U ,0U), "");
    static_assert(testComparisons6Values<month>(0U, 1U), "");

//  Some 'ok' values as well
    static_assert(testComparisons6Values<month>( 5U,  5U), "");
    static_assert(testComparisons6Values<month>( 5U, 10U), "");

    for (unsigned i = 1; i < 10; ++i)
        for (unsigned j = 10; j < 10; ++j)
            assert(testComparisons6Values<month>(i, j));
}
