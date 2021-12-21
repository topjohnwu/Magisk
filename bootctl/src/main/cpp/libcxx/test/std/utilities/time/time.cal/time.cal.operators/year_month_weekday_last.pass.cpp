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
// class year_month_weekday_last;

// constexpr year_month_weekday_last
//   operator/(const year_month& ym, const weekday_last& wdl) noexcept;
// Returns: {ym.year(), ym.month(), wdl}.
//
// constexpr year_month_weekday_last
//   operator/(const year& y, const month_weekday_last& mwdl) noexcept;
// Returns: {y, mwdl.month(), mwdl.weekday_last()}.
//
// constexpr year_month_weekday_last
//   operator/(int y, const month_weekday_last& mwdl) noexcept;
// Returns: year(y) / mwdl.
//
// constexpr year_month_weekday_last
//   operator/(const month_weekday_last& mwdl, const year& y) noexcept;
// Returns: y / mwdl.
//
// constexpr year_month_weekday_last
//   operator/(const month_weekday_last& mwdl, int y) noexcept;
// Returns: year(y) / mwdl.



#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using year_month         = std::chrono::year_month;
    using year               = std::chrono::year;
    using month              = std::chrono::month;
    using weekday            = std::chrono::weekday;
    using weekday_last       = std::chrono::weekday_last;
    using month_weekday_last = std::chrono::month_weekday_last;
    using year_month_weekday_last = std::chrono::year_month_weekday_last;

    constexpr weekday Tuesday = std::chrono::Tuesday;
    constexpr month February = std::chrono::February;

    { // operator/(const year_month& ym, const weekday_last& wdl) (and switched)
        constexpr year_month Feb2018{year{2018}, February};

        ASSERT_NOEXCEPT (                                  Feb2018/weekday_last{Tuesday});
        ASSERT_SAME_TYPE(year_month_weekday_last, decltype(Feb2018/weekday_last{Tuesday}));

        static_assert((Feb2018/weekday_last{Tuesday}).year()    == year{2018}, "");
        static_assert((Feb2018/weekday_last{Tuesday}).month()   == February,   "");
        static_assert((Feb2018/weekday_last{Tuesday}).weekday() == Tuesday,    "");

        for (int i = 1000; i < 1010; ++i)
            for (unsigned j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 6; ++k)
                {
                    year y{i};
                    month m{j};
                    weekday wd{k};
                    year_month_weekday_last ymwdl = year_month{y,m}/weekday_last{wd};
                    assert(ymwdl.year()    == y);
                    assert(ymwdl.month()   == m);
                    assert(ymwdl.weekday() == wd);
                }
    }


    { // operator/(const year& y, const month_weekday_last& mwdl) (and switched)
        constexpr month_weekday_last FebLastTues{February, weekday_last{Tuesday}};

        ASSERT_NOEXCEPT (                                  year{2018}/FebLastTues);
        ASSERT_SAME_TYPE(year_month_weekday_last, decltype(year{2018}/FebLastTues));
        ASSERT_NOEXCEPT (                                  FebLastTues/year{2018});
        ASSERT_SAME_TYPE(year_month_weekday_last, decltype(FebLastTues/year{2018}));


        static_assert((year{2018}/FebLastTues).year()    == year{2018}, "");
        static_assert((year{2018}/FebLastTues).month()   == February,   "");
        static_assert((year{2018}/FebLastTues).weekday() == Tuesday,    "");
        static_assert((FebLastTues/year{2018}).year()    == year{2018}, "");
        static_assert((FebLastTues/year{2018}).month()   == February,   "");
        static_assert((FebLastTues/year{2018}).weekday() == Tuesday,    "");


        for (int i = 1000; i < 1010; ++i)
            for (unsigned j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 6; ++k)
                {
                    year y{i};
                    month m{j};
                    weekday wd{k};
                    year_month_weekday_last ymwdl1 = y/month_weekday_last{m, weekday_last{wd}};
                    year_month_weekday_last ymwdl2 = month_weekday_last{m, weekday_last{wd}}/y;
                    assert(ymwdl1.year()    == y);
                    assert(ymwdl2.year()    == y);
                    assert(ymwdl1.month()   == m);
                    assert(ymwdl2.month()   == m);
                    assert(ymwdl1.weekday() == wd);
                    assert(ymwdl2.weekday() == wd);
                    assert(ymwdl1 == ymwdl2);
                }
    }


    { // operator/(int y, const month_weekday_last& mwdl) (and switched)
        constexpr month_weekday_last FebLastTues{February, weekday_last{Tuesday}};

        ASSERT_NOEXCEPT (                                  2018/FebLastTues);
        ASSERT_SAME_TYPE(year_month_weekday_last, decltype(2018/FebLastTues));
        ASSERT_NOEXCEPT (                                  FebLastTues/2018);
        ASSERT_SAME_TYPE(year_month_weekday_last, decltype(FebLastTues/2018));


        static_assert((2018/FebLastTues).year()    == year{2018}, "");
        static_assert((2018/FebLastTues).month()   == February,   "");
        static_assert((2018/FebLastTues).weekday() == Tuesday,    "");
        static_assert((FebLastTues/2018).year()    == year{2018}, "");
        static_assert((FebLastTues/2018).month()   == February,   "");
        static_assert((FebLastTues/2018).weekday() == Tuesday,    "");


        for (int i = 1000; i < 1010; ++i)
            for (unsigned j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 6; ++k)
                {
                    year y{i};
                    month m{j};
                    weekday wd{k};
                    year_month_weekday_last ymwdl1 = i/month_weekday_last{m, weekday_last{wd}};
                    year_month_weekday_last ymwdl2 = month_weekday_last{m, weekday_last{wd}}/i;
                    assert(ymwdl1.year()    == y);
                    assert(ymwdl2.year()    == y);
                    assert(ymwdl1.month()   == m);
                    assert(ymwdl2.month()   == m);
                    assert(ymwdl1.weekday() == wd);
                    assert(ymwdl2.weekday() == wd);
                    assert(ymwdl1 == ymwdl2);
                }
    }
}
