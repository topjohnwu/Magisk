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

// constexpr day operator-(const day& x, const days& y) noexcept;
//   Returns: x + -y.
//
// constexpr days operator-(const day& x, const day& y) noexcept;
//   Returns: days{int(unsigned{x}) - int(unsigned{y}).


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename D, typename Ds>
constexpr bool testConstexpr()
{
    D d{23};
    Ds offset{6};
    if (d - offset != D{17}) return false;
    if (d - D{17} != offset) return false;
    return true;
}

int main()
{
    using day  = std::chrono::day;
    using days = std::chrono::days;

    ASSERT_NOEXCEPT(std::declval<day>() - std::declval<days>());
    ASSERT_NOEXCEPT(std::declval<day>() - std::declval<day>());

    ASSERT_SAME_TYPE(day,  decltype(std::declval<day>() - std::declval<days>()));
    ASSERT_SAME_TYPE(days, decltype(std::declval<day>() - std::declval<day>()));

    static_assert(testConstexpr<day, days>(), "");

    day dy{12};
    for (unsigned i = 0; i <= 10; ++i)
    {
        day d1   = dy - days{i};
        days off = dy - day {i};
        assert(static_cast<unsigned>(d1) == 12 - i);
        assert(off.count() == static_cast<int>(12 - i)); // days is signed
    }
}
