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

// constexpr month operator-(const month& x, const months& y) noexcept;
//   Returns: x + -y.
//
// constexpr months operator-(const month& x, const month& y) noexcept;
//   Returns: If x.ok() == true and y.ok() == true, returns a value m in the range
//   [months{0}, months{11}] satisfying y + m == x.
//   Otherwise the value returned is unspecified.
//   [Example: January - February == months{11}. —end example]

extern "C" int printf(const char *, ...);

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename M, typename Ms>
constexpr bool testConstexpr()
{
    {
    M m{5};
    Ms offset{3};
    if (m - offset != M{2}) return false;
    if (m - M{2} != offset) return false;
    }

//  Check the example
    if (M{1} - M{2} != Ms{11}) return false;
    return true;
}

#include <iostream>

int main()
{
    using month  = std::chrono::month;
    using months = std::chrono::months;

    ASSERT_NOEXCEPT(std::declval<month>() - std::declval<months>());
    ASSERT_NOEXCEPT(std::declval<month>() - std::declval<month>());

    ASSERT_SAME_TYPE(month , decltype(std::declval<month>() - std::declval<months>()));
    ASSERT_SAME_TYPE(months, decltype(std::declval<month>() - std::declval<month> ()));

static_assert(testConstexpr<month, months>(), "");

    month m{6};
    for (unsigned i = 1; i <= 12; ++i)
    {
        month m1   = m - months{i};
//      months off = m - month {i};
        int exp = 6 - i;
        if (exp < 1)
            exp += 12;
        assert(static_cast<unsigned>(m1) == static_cast<unsigned>(exp));
//          assert(off.count()               == static_cast<unsigned>(exp));
    }
}
