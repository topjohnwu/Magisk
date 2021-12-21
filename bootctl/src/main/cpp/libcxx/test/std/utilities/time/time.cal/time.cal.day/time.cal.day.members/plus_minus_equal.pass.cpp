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

// constexpr day& operator+=(const days& d) noexcept;
// constexpr day& operator-=(const days& d) noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename D, typename Ds>
constexpr bool testConstexpr()
{
    D d1{1};
    if (static_cast<unsigned>(d1 += Ds{ 1}) !=  2) return false;
    if (static_cast<unsigned>(d1 += Ds{ 2}) !=  4) return false;
    if (static_cast<unsigned>(d1 += Ds{22}) != 26) return false;
    if (static_cast<unsigned>(d1 -= Ds{ 1}) != 25) return false;
    if (static_cast<unsigned>(d1 -= Ds{ 2}) != 23) return false;
    if (static_cast<unsigned>(d1 -= Ds{22}) !=  1) return false;
    return true;
}

int main()
{
    using day  = std::chrono::day;
    using days = std::chrono::days;

    ASSERT_NOEXCEPT(std::declval<day&>() += std::declval<days>());
    ASSERT_NOEXCEPT(std::declval<day&>() -= std::declval<days>());

    ASSERT_SAME_TYPE(day&, decltype(std::declval<day&>() += std::declval<days>()));
    ASSERT_SAME_TYPE(day&, decltype(std::declval<day&>() -= std::declval<days>()));

    static_assert(testConstexpr<day, days>(), "");

    for (unsigned i = 0; i <= 10; ++i)
    {
        day day(i);
        assert(static_cast<unsigned>(day += days{22}) == i + 22);
        assert(static_cast<unsigned>(day)             == i + 22);
        assert(static_cast<unsigned>(day -= days{12}) == i + 10);
        assert(static_cast<unsigned>(day)             == i + 10);
    }
}
