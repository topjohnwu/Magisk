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
// class year_month_weekday;

// constexpr year_month_weekday operator+(const year_month_weekday& ymd, const months& dm) noexcept;
//   Returns: (ymd.year() / ymd.month() + dm) / ymd.day().
//
// constexpr year_month_weekday operator+(const months& dm, const year_month_weekday& ymd) noexcept;
//   Returns: ymd + dm.
//
//
// constexpr year_month_weekday operator+(const year_month_weekday& ymd, const years& dy) noexcept;
//   Returns: (ymd.year() + dy) / ymd.month() / ymd.day().
//
// constexpr year_month_weekday operator+(const years& dy, const year_month_weekday& ymd) noexcept;
//   Returns: ym + dm.



#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

constexpr bool testConstexprYears(std::chrono::year_month_weekday ym)
{
    std::chrono::years offset{23};
    if (static_cast<int>((ym         ).year()) !=  1) return false;
    if (static_cast<int>((ym + offset).year()) != 24) return false;
    if (static_cast<int>((offset + ym).year()) != 24) return false;
    return true;
}


constexpr bool testConstexprMonths(std::chrono::year_month_weekday ym)
{
    std::chrono::months offset{6};
    if (static_cast<unsigned>((ym         ).month()) !=  1) return false;
    if (static_cast<unsigned>((ym + offset).month()) !=  7) return false;
    if (static_cast<unsigned>((offset + ym).month()) !=  7) return false;
    return true;
}


int main()
{
    using year               = std::chrono::year;
    using month              = std::chrono::month;
    using weekday            = std::chrono::weekday;
    using weekday_indexed    = std::chrono::weekday_indexed;
    using year_month_weekday = std::chrono::year_month_weekday;
    using years              = std::chrono::years;
    using months             = std::chrono::months;

    constexpr weekday Tuesday = std::chrono::Tuesday;
    constexpr month January = std::chrono::January;

    {   // year_month_weekday + months (and switched)
    ASSERT_NOEXCEPT(std::declval<year_month_weekday>() + std::declval<months>());
    ASSERT_NOEXCEPT(std::declval<months>() + std::declval<year_month_weekday>());

    ASSERT_SAME_TYPE(year_month_weekday, decltype(std::declval<year_month_weekday>() + std::declval<months>()));
    ASSERT_SAME_TYPE(year_month_weekday, decltype(std::declval<months>() + std::declval<year_month_weekday>()));

    static_assert(testConstexprMonths(year_month_weekday{year{1}, January, weekday_indexed{Tuesday, 1}}), "");

    year_month_weekday ym{year{1234}, January, weekday_indexed{Tuesday, 3}};
    for (int i = 0; i <= 10; ++i)  // TODO test wrap-around
    {
        year_month_weekday ym1 = ym + months{i};
        year_month_weekday ym2 = months{i} + ym;
        assert(static_cast<int>(ym1.year()) == 1234);
        assert(static_cast<int>(ym2.year()) == 1234);
        assert(ym1.month()   == month(1 + i));
        assert(ym2.month()   == month(1 + i));
        assert(ym1.weekday() == Tuesday);
        assert(ym2.weekday() == Tuesday);
        assert(ym1.index()   == 3);
        assert(ym2.index()   == 3);
        assert(ym1 == ym2);
    }
    }

    {   // year_month_weekday + years (and switched)
    ASSERT_NOEXCEPT(std::declval<year_month_weekday>() + std::declval<years>());
    ASSERT_NOEXCEPT(std::declval<years>() + std::declval<year_month_weekday>());

    ASSERT_SAME_TYPE(year_month_weekday, decltype(std::declval<year_month_weekday>() + std::declval<years>()));
    ASSERT_SAME_TYPE(year_month_weekday, decltype(std::declval<years>() + std::declval<year_month_weekday>()));

    static_assert(testConstexprYears (year_month_weekday{year{1}, January, weekday_indexed{Tuesday, 1}}), "");

    year_month_weekday ym{year{1234}, std::chrono::January, weekday_indexed{Tuesday, 3}};
    for (int i = 0; i <= 10; ++i)
    {
        year_month_weekday ym1 = ym + years{i};
        year_month_weekday ym2 = years{i} + ym;
        assert(static_cast<int>(ym1.year()) == i + 1234);
        assert(static_cast<int>(ym2.year()) == i + 1234);
        assert(ym1.month()   == January);
        assert(ym2.month()   == January);
        assert(ym1.weekday() == Tuesday);
        assert(ym2.weekday() == Tuesday);
        assert(ym1.index()   == 3);
        assert(ym2.index()   == 3);
        assert(ym1 == ym2);
    }
    }

}
