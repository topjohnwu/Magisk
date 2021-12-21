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

// constexpr year_month_weekday
//   operator/(const year_month& ym, const weekday_indexed& wdi) noexcept;
// Returns: {ym.year(), ym.month(), wdi}.
//
// constexpr year_month_weekday
//   operator/(const year& y, const month_weekday& mwd) noexcept;
// Returns: {y, mwd.month(), mwd.weekday_indexed()}.
//
// constexpr year_month_weekday
//   operator/(int y, const month_weekday& mwd) noexcept;
// Returns: year(y) / mwd.
//
// constexpr year_month_weekday
//   operator/(const month_weekday& mwd, const year& y) noexcept;
// Returns: y / mwd.
//
// constexpr year_month_weekday
//   operator/(const month_weekday& mwd, int y) noexcept;
// Returns: year(y) / mwd.

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using year               = std::chrono::year;
    using year_month         = std::chrono::year_month;
    using month_weekday      = std::chrono::month_weekday;
    using year_month_weekday = std::chrono::year_month_weekday;
    using month              = std::chrono::month;
    using weekday            = std::chrono::weekday;
    using weekday            = std::chrono::weekday;
    using weekday_indexed    = std::chrono::weekday_indexed;

    constexpr weekday Tuesday = std::chrono::Tuesday;
    constexpr month February = std::chrono::February;


    { // operator/(const year_month& ym, const weekday_indexed& wdi)
        constexpr year_month Feb2018{year{2018}, February};

        ASSERT_NOEXCEPT (                             Feb2018/weekday_indexed{Tuesday, 2});
        ASSERT_SAME_TYPE(year_month_weekday, decltype(Feb2018/weekday_indexed{Tuesday, 2}));

        static_assert((Feb2018/weekday_indexed{Tuesday, 2}).year()    == year{2018}, "" );
        static_assert((Feb2018/weekday_indexed{Tuesday, 2}).month()   == February, "" );
        static_assert((Feb2018/weekday_indexed{Tuesday, 2}).weekday() == Tuesday,  "" );

        for (int i = 1000; i < 1010; ++i)
            for (int j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 6; ++k)
                    for (unsigned l = 1; l <= 5; ++l)
                    {
                        year y(i);
                        month m(j);
                        weekday wd{k};
                        year_month_weekday ymd = year_month{y,m}/weekday_indexed{wd,l};
                        assert(ymd.year()    == y);
                        assert(ymd.month()   == m);
                        assert(ymd.weekday() == wd);
                    }
    }

    { // operator/(const year& y, const month_weekday& mwd) (and switched)
        constexpr month_weekday Feb1stTues{February, weekday_indexed{Tuesday, 1}};
        ASSERT_NOEXCEPT (                             year{2018}/Feb1stTues);
        ASSERT_SAME_TYPE(year_month_weekday, decltype(year{2018}/Feb1stTues));
        ASSERT_NOEXCEPT (                             Feb1stTues/year{2018});
        ASSERT_SAME_TYPE(year_month_weekday, decltype(Feb1stTues/year{2018}));

        static_assert((year{2018}/Feb1stTues).year()    == year{2018}, "" );
        static_assert((year{2018}/Feb1stTues).month()   == February, "" );
        static_assert((year{2018}/Feb1stTues).weekday() == Tuesday,  "" );

        for (int i = 1000; i < 1010; ++i)
            for (int j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 6; ++k)
                    for (unsigned l = 1; l <= 5; ++l)
                    {
                        year y(i);
                        month m(j);
                        weekday wd{k};
                        month_weekday mwd{m, weekday_indexed{weekday{k}, l}};
                        year_month_weekday ymd1 = y/mwd;
                        year_month_weekday ymd2 = mwd/y;
                        assert(ymd1.year()    == y);
                        assert(ymd2.year()    == y);
                        assert(ymd1.month()   == m);
                        assert(ymd2.month()   == m);
                        assert(ymd1.weekday() == wd);
                        assert(ymd2.weekday() == wd);
                        assert(ymd1 == ymd2);
                    }
    }


    { // operator/(int y, const month_weekday& mwd) (and switched)
        constexpr month_weekday Feb1stTues{February, weekday_indexed{Tuesday, 1}};
        ASSERT_NOEXCEPT (                             2018/Feb1stTues);
        ASSERT_SAME_TYPE(year_month_weekday, decltype(2018/Feb1stTues));
        ASSERT_NOEXCEPT (                             Feb1stTues/2018);
        ASSERT_SAME_TYPE(year_month_weekday, decltype(Feb1stTues/2018));

        static_assert((2018/Feb1stTues).year()    == year{2018}, "" );
        static_assert((2018/Feb1stTues).month()   == February, "" );
        static_assert((2018/Feb1stTues).weekday() == Tuesday,  "" );

        for (int i = 1000; i < 1010; ++i)
            for (int j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 6; ++k)
                    for (unsigned l = 1; l <= 5; ++l)
                    {
                        year y(i);
                        month m(j);
                        weekday wd{k};
                        month_weekday mwd{m, weekday_indexed{weekday{k}, l}};
                        year_month_weekday ymd1 = i/mwd;
                        year_month_weekday ymd2 = mwd/i;
                        assert(ymd1.year()    == y);
                        assert(ymd2.year()    == y);
                        assert(ymd1.month()   == m);
                        assert(ymd2.month()   == m);
                        assert(ymd1.weekday() == wd);
                        assert(ymd2.weekday() == wd);
                        assert(ymd1 == ymd2);
                    }
     }
}
