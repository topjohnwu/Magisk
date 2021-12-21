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

// constexpr month& operator+=(const month& d) noexcept;
// constexpr month& operator-=(const month& d) noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename M, typename Ms>
constexpr bool testConstexpr()
{
    M m1{1};
    if (static_cast<unsigned>(m1 += Ms{ 1}) !=  2) return false;
    if (static_cast<unsigned>(m1 += Ms{ 2}) !=  4) return false;
    if (static_cast<unsigned>(m1 += Ms{ 8}) != 12) return false;
    if (static_cast<unsigned>(m1 -= Ms{ 1}) != 11) return false;
    if (static_cast<unsigned>(m1 -= Ms{ 2}) !=  9) return false;
    if (static_cast<unsigned>(m1 -= Ms{ 8}) !=  1) return false;
    return true;
}

int main()
{
    using month  = std::chrono::month;
    using months = std::chrono::months;

    ASSERT_NOEXCEPT(std::declval<month&>() += std::declval<months&>());
    ASSERT_NOEXCEPT(std::declval<month&>() -= std::declval<months&>());
    ASSERT_SAME_TYPE(month&, decltype(std::declval<month&>() += std::declval<months&>()));
    ASSERT_SAME_TYPE(month&, decltype(std::declval<month&>() -= std::declval<months&>()));

    static_assert(testConstexpr<month, months>(), "");

    for (unsigned i = 1; i <= 10; ++i)
    {
        month month(i);
        int exp = i + 10;
        while (exp > 12)
            exp -= 12;
        assert(static_cast<unsigned>(month += months{10}) == static_cast<unsigned>(exp));
        assert(static_cast<unsigned>(month)               == static_cast<unsigned>(exp));
    }

    for (unsigned i = 1; i <= 10; ++i)
    {
        month month(i);
        int exp = i - 9;
        while (exp < 1)
            exp += 12;
        assert(static_cast<unsigned>(month -= months{ 9}) == static_cast<unsigned>(exp));
        assert(static_cast<unsigned>(month)               == static_cast<unsigned>(exp));
    }
}
