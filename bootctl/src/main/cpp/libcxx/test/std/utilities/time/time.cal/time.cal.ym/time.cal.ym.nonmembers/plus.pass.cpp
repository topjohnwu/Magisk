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
// class year_month;

// constexpr year_month operator+(const year_month& ym, const years& dy) noexcept;
// Returns: (ym.year() + dy) / ym.month().
//
// constexpr year_month operator+(const years& dy, const year_month& ym) noexcept;
// Returns: ym + dy.
//
// constexpr year_month operator+(const year_month& ym, const months& dm) noexcept;
// Returns: A year_month value z such that z - ym == dm.
// Complexity: O(1) with respect to the value of dm.
//
// constexpr year_month operator+(const months& dm, const year_month& ym) noexcept;
// Returns: ym + dm.



#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

constexpr bool testConstexprYears(std::chrono::year_month ym)
{
    std::chrono::years offset{23};
    if (static_cast<int>((ym         ).year()) !=  1) return false;
    if (static_cast<int>((ym + offset).year()) != 24) return false;
    if (static_cast<int>((offset + ym).year()) != 24) return false;
    return true;
}


constexpr bool testConstexprMonths(std::chrono::year_month ym)
{
    std::chrono::months offset{6};
    if (static_cast<unsigned>((ym         ).month()) !=  1) return false;
    if (static_cast<unsigned>((ym + offset).month()) !=  7) return false;
    if (static_cast<unsigned>((offset + ym).month()) !=  7) return false;
    return true;
}


int main()
{
    using year       = std::chrono::year;
    using years      = std::chrono::years;
    using month      = std::chrono::month;
    using months     = std::chrono::months;
    using year_month = std::chrono::year_month;

    {   // year_month + years
    ASSERT_NOEXCEPT(std::declval<year_month>() + std::declval<years>());
    ASSERT_NOEXCEPT(std::declval<years>() + std::declval<year_month>());

    ASSERT_SAME_TYPE(year_month, decltype(std::declval<year_month>() + std::declval<years>()));
    ASSERT_SAME_TYPE(year_month, decltype(std::declval<years>() + std::declval<year_month>()));

    static_assert(testConstexprYears (year_month{year{1}, month{1}}), "");

    year_month ym{year{1234}, std::chrono::January};
    for (int i = 0; i <= 10; ++i)
    {
        year_month ym1 = ym + years{i};
        year_month ym2 = years{i} + ym;
        assert(static_cast<int>(ym1.year()) == i + 1234);
        assert(static_cast<int>(ym2.year()) == i + 1234);
        assert(ym1.month() == std::chrono::January);
        assert(ym2.month() == std::chrono::January);
        assert(ym1 == ym2);
    }
    }

    {   // year_month + months
    ASSERT_NOEXCEPT(std::declval<year_month>() + std::declval<months>());
    ASSERT_NOEXCEPT(std::declval<months>() + std::declval<year_month>());

    ASSERT_SAME_TYPE(year_month, decltype(std::declval<year_month>() + std::declval<months>()));
    ASSERT_SAME_TYPE(year_month, decltype(std::declval<months>() + std::declval<year_month>()));

    static_assert(testConstexprMonths(year_month{year{1}, month{1}}), "");

    year_month ym{year{1234}, std::chrono::January};
    for (int i = 0; i <= 10; ++i)  // TODO test wrap-around
    {
        year_month ym1 = ym + months{i};
        year_month ym2 = months{i} + ym;
        assert(static_cast<int>(ym1.year()) == 1234);
        assert(static_cast<int>(ym2.year()) == 1234);
        assert(ym1.month() == month(1 + i));
        assert(ym2.month() == month(1 + i));
        assert(ym1 == ym2);
    }
    }
}
