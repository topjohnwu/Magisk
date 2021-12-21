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

// constexpr month operator+(const month& x, const months& y) noexcept;
//   Returns: month(int{x} + y.count()).
//
// constexpr month operator+(const months& x, const month& y) noexcept;
//   Returns:
//     month{modulo(static_cast<long long>(int{x}) + (y.count() - 1), 12) + 1}
//   where modulo(n, 12) computes the remainder of n divided by 12 using Euclidean division.
//   [Note: Given a divisor of 12, Euclidean division truncates towards negative infinity
//   and always produces a remainder in the range of [0, 11].
//   Assuming no overflow in the signed summation, this operation results in a month
//   holding a value in the range [1, 12] even if !x.ok(). —end note]
//   [Example: February + months{11} == January. —end example]



#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename M, typename Ms>
constexpr bool testConstexpr()
{
    M m{1};
    Ms offset{4};
    if (m + offset != M{5}) return false;
    if (offset + m != M{5}) return false;
//  Check the example
    if (M{2} + Ms{11} != M{1}) return false;
    return true;
}

int main()
{
    using month  = std::chrono::month;
    using months = std::chrono::months;

    ASSERT_NOEXCEPT(std::declval<month>() + std::declval<months>());
    ASSERT_NOEXCEPT(std::declval<months>() + std::declval<month>());

    ASSERT_SAME_TYPE(month, decltype(std::declval<month>()  + std::declval<months>()));
    ASSERT_SAME_TYPE(month, decltype(std::declval<months>() + std::declval<month>() ));

    static_assert(testConstexpr<month, months>(), "");

    month my{2};
    for (unsigned i = 0; i <= 15; ++i)
    {
        month m1 = my + months{i};
        month m2 = months{i} + my;
        assert(m1 == m2);
        unsigned exp = i + 2;
        while (exp > 12)
            exp -= 12;
        assert(static_cast<unsigned>(m1) == exp);
        assert(static_cast<unsigned>(m2) == exp);
    }
}
