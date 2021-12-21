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

// constexpr year_month_weekday operator-(const year_month_weekday& ymwd, const months& dm) noexcept;
//   Returns: ymwd + (-dm).
//
// constexpr year_month_weekday operator-(const year_month_weekday& ymwd, const years& dy) noexcept;
//   Returns: ymwd + (-dy).


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

#include <iostream>

constexpr bool testConstexprYears ()
{
    std::chrono::year_month_weekday ym0{std::chrono::year{1234}, std::chrono::January, std::chrono::weekday_indexed{std::chrono::Tuesday, 1}};
    std::chrono::year_month_weekday ym1 = ym0 - std::chrono::years{10};
    return
        ym1.year()    == std::chrono::year{1234-10}
     && ym1.month()   == std::chrono::January
     && ym1.weekday() == std::chrono::Tuesday
     && ym1.index()   == 1
        ;
}

constexpr bool testConstexprMonths ()
{
    std::chrono::year_month_weekday ym0{std::chrono::year{1234}, std::chrono::November, std::chrono::weekday_indexed{std::chrono::Tuesday, 1}};
    std::chrono::year_month_weekday ym1 = ym0 - std::chrono::months{6};
    return
        ym1.year()    == std::chrono::year{1234}
     && ym1.month()   == std::chrono::May
     && ym1.weekday() == std::chrono::Tuesday
     && ym1.index()   == 1
        ;
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

    constexpr month November  = std::chrono::November;
    constexpr weekday Tuesday = std::chrono::Tuesday;

    {  // year_month_weekday - years
    ASSERT_NOEXCEPT(                              std::declval<year_month_weekday>() - std::declval<years>());
    ASSERT_SAME_TYPE(year_month_weekday, decltype(std::declval<year_month_weekday>() - std::declval<years>()));

    static_assert(testConstexprYears(), "");

    year_month_weekday ym{year{1234}, November, weekday_indexed{Tuesday, 1}};
    for (int i = 0; i <= 10; ++i)
    {
        year_month_weekday ym1 = ym - years{i};
        assert(static_cast<int>(ym1.year()) == 1234 - i);
        assert(ym1.month()   == November);
        assert(ym1.weekday() == Tuesday);
        assert(ym1.index()   == 1);
    }
    }

    {  // year_month_weekday - months
    ASSERT_NOEXCEPT(                              std::declval<year_month_weekday>() - std::declval<months>());
    ASSERT_SAME_TYPE(year_month_weekday, decltype(std::declval<year_month_weekday>() - std::declval<months>()));

    static_assert(testConstexprMonths(), "");

    year_month_weekday ym{year{1234}, November, weekday_indexed{Tuesday, 2}};
    for (unsigned i = 1; i <= 10; ++i)
    {
        year_month_weekday ym1 = ym - months{i};
        assert(ym1.year()    == year{1234});
        assert(ym1.month()   == month{11-i});
        assert(ym1.weekday() == Tuesday);
        assert(ym1.index()   == 2);
    }
    }
}
