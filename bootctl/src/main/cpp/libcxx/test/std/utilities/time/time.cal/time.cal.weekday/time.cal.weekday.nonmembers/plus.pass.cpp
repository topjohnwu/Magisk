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

// constexpr weekday operator+(const days& x, const weekday& y) noexcept;
//   Returns: weekday(int{x} + y.count()).
//
// constexpr weekday operator+(const weekday& x, const days& y) noexcept;
//   Returns:
//      weekday{modulo(static_cast<long long>(unsigned{x}) + y.count(), 7)}
//   where modulo(n, 7) computes the remainder of n divided by 7 using Euclidean division.
//   [Note: Given a divisor of 12, Euclidean division truncates towards negative infinity
//   and always produces a remainder in the range of [0, 6].
//   Assuming no overflow in the signed summation, this operation results in a weekday
//   holding a value in the range [0, 6] even if !x.ok(). —end note]
//   [Example: Monday + days{6} == Sunday. —end example]



#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "../../euclidian.h"

template <typename M, typename Ms>
constexpr bool testConstexpr()
{
    M m{1};
    Ms offset{4};
    if (m + offset != M{5}) return false;
    if (offset + m != M{5}) return false;
//  Check the example
    if (M{1} + Ms{6} != M{0}) return false;
    return true;
}

int main()
{
    using weekday = std::chrono::weekday;
    using days    = std::chrono::days;

    ASSERT_NOEXCEPT(                   std::declval<weekday>() + std::declval<days>());
    ASSERT_SAME_TYPE(weekday, decltype(std::declval<weekday>() + std::declval<days>()));

    ASSERT_NOEXCEPT(                   std::declval<days>() + std::declval<weekday>());
    ASSERT_SAME_TYPE(weekday, decltype(std::declval<days>() + std::declval<weekday>()));

    static_assert(testConstexpr<weekday, days>(), "");

    for (unsigned i = 0; i <= 6; ++i)
        for (unsigned j = 0; j <= 6; ++j)
        {
            weekday wd1 = weekday{i} + days{j};
            weekday wd2 = days{j} + weekday{i};
            assert(wd1 == wd2);
            assert((static_cast<unsigned>(wd1) == euclidian_addition<unsigned, 0, 6>(i, j)));
            assert((static_cast<unsigned>(wd2) == euclidian_addition<unsigned, 0, 6>(i, j)));
        }
}
