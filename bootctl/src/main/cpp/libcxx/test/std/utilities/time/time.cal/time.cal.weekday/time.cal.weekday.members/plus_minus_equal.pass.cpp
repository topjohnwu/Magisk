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

// constexpr weekday& operator+=(const days& d) noexcept;
// constexpr weekday& operator-=(const days& d) noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "../../euclidian.h"

template <typename M, typename Ms>
constexpr bool testConstexpr()
{
    M m1{1};
    if (static_cast<unsigned>(m1 += Ms{ 1}) !=  2) return false;
    if (static_cast<unsigned>(m1 += Ms{ 2}) !=  4) return false;
    if (static_cast<unsigned>(m1 += Ms{ 4}) !=  1) return false;
    if (static_cast<unsigned>(m1 -= Ms{ 1}) !=  0) return false;
    if (static_cast<unsigned>(m1 -= Ms{ 2}) !=  5) return false;
    if (static_cast<unsigned>(m1 -= Ms{ 4}) !=  1) return false;
    return true;
}

int main()
{
    using weekday = std::chrono::weekday;
    using days    = std::chrono::days;

    ASSERT_NOEXCEPT(                    std::declval<weekday&>() += std::declval<days&>());
    ASSERT_SAME_TYPE(weekday&, decltype(std::declval<weekday&>() += std::declval<days&>()));

    ASSERT_NOEXCEPT(                    std::declval<weekday&>() -= std::declval<days&>());
    ASSERT_SAME_TYPE(weekday&, decltype(std::declval<weekday&>() -= std::declval<days&>()));

    static_assert(testConstexpr<weekday, days>(), "");

    for (unsigned i = 0; i <= 6; ++i)
    {
        weekday wd(i);
        assert((static_cast<unsigned>(wd += days{3}) == euclidian_addition<unsigned, 0, 6>(i, 3)));
        assert((static_cast<unsigned>(wd)            == euclidian_addition<unsigned, 0, 6>(i, 3)));
    }

    for (unsigned i = 0; i <= 6; ++i)
    {
        weekday wd(i);
        assert((static_cast<unsigned>(wd -= days{4}) == euclidian_subtraction<unsigned, 0, 6>(i, 4)));
        assert((static_cast<unsigned>(wd)            == euclidian_subtraction<unsigned, 0, 6>(i, 4)));
    }
}
