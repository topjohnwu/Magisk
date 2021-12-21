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

// constexpr year_month_weekday& operator+=(const months& m) noexcept;
// constexpr year_month_weekday& operator-=(const months& m) noexcept;

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename D, typename Ds>
constexpr bool testConstexpr(D d1)
{
    if (static_cast<unsigned>((d1          ).month()) !=  1) return false;
    if (static_cast<unsigned>((d1 += Ds{ 1}).month()) !=  2) return false;
    if (static_cast<unsigned>((d1 += Ds{ 2}).month()) !=  4) return false;
    if (static_cast<unsigned>((d1 += Ds{12}).month()) !=  4) return false;
    if (static_cast<unsigned>((d1 -= Ds{ 1}).month()) !=  3) return false;
    if (static_cast<unsigned>((d1 -= Ds{ 2}).month()) !=  1) return false;
    if (static_cast<unsigned>((d1 -= Ds{12}).month()) !=  1) return false;
    return true;
}

int main()
{
    using year               = std::chrono::year;
    using month              = std::chrono::month;
    using weekday            = std::chrono::weekday;
    using weekday_indexed    = std::chrono::weekday_indexed;
    using year_month_weekday = std::chrono::year_month_weekday;
    using months             = std::chrono::months;


    ASSERT_NOEXCEPT(                               std::declval<year_month_weekday&>() += std::declval<months>());
    ASSERT_SAME_TYPE(year_month_weekday&, decltype(std::declval<year_month_weekday&>() += std::declval<months>()));

    ASSERT_NOEXCEPT(                               std::declval<year_month_weekday&>() -= std::declval<months>());
    ASSERT_SAME_TYPE(year_month_weekday&, decltype(std::declval<year_month_weekday&>() -= std::declval<months>()));

    constexpr weekday Tuesday = std::chrono::Tuesday;
    static_assert(testConstexpr<year_month_weekday, months>(year_month_weekday{year{1234}, month{1}, weekday_indexed{Tuesday, 2}}), "");

    for (unsigned i = 0; i <= 10; ++i)
    {
        year y{1234};
        year_month_weekday ymwd(y, month{i}, weekday_indexed{Tuesday, 2});

        assert(static_cast<unsigned>((ymwd += months{2}).month()) == i + 2);
        assert(ymwd.year()     == y);
        assert(ymwd.weekday()  == Tuesday);
        assert(ymwd.index()    == 2);

        assert(static_cast<unsigned>((ymwd             ).month()) == i + 2);
        assert(ymwd.year()     == y);
        assert(ymwd.weekday()  == Tuesday);
        assert(ymwd.index()    == 2);

        assert(static_cast<unsigned>((ymwd -= months{1}).month()) == i + 1);
        assert(ymwd.year()     == y);
        assert(ymwd.weekday()  == Tuesday);
        assert(ymwd.index()    == 2);

        assert(static_cast<unsigned>((ymwd             ).month()) == i + 1);
        assert(ymwd.year()     == y);
        assert(ymwd.weekday()  == Tuesday);
        assert(ymwd.index()    == 2);
    }
}
