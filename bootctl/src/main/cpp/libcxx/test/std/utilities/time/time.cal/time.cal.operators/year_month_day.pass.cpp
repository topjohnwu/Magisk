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
// class year_month_day;

// constexpr year_month_day
//   operator/(const year_month& ym, const day& d) noexcept;
// Returns: {ym.year(), ym.month(), d}.
//
// constexpr year_month_day
//   operator/(const year_month& ym, int d) noexcept;
// Returns: ym / day(d).
//
// constexpr year_month_day
//   operator/(const year& y, const month_day& md) noexcept;
// Returns: y / md.month() / md.day().
//
// constexpr year_month_day
//   operator/(int y, const month_day& md) noexcept;
// Returns: year(y) / md.
//
// constexpr year_month_day
//   operator/(const month_day& md, const year& y) noexcept;
// Returns: y / md.
//
// constexpr year_month_day
//   operator/(const month_day& md, int y) noexcept;
// Returns: year(y) / md.


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using year           = std::chrono::year;
    using month          = std::chrono::month;
    using day            = std::chrono::day;
    using year_month     = std::chrono::year_month;
    using month_day      = std::chrono::month_day;
    using year_month_day = std::chrono::year_month_day;

    constexpr month February = std::chrono::February;
    constexpr year_month Feb2018{year{2018}, February};

    { // operator/(const year_month& ym, const day& d)
        ASSERT_NOEXCEPT (                         Feb2018/day{2});
        ASSERT_SAME_TYPE(year_month_day, decltype(Feb2018/day{2}));

        static_assert((Feb2018/day{2}).month() == February, "");
        static_assert((Feb2018/day{2}).day()   == day{2},   "");

        for (int i = 1000; i < 1010; ++i)
            for (int j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 28; ++k)
                {
                    year y(i);
                    month m(j);
                    day d(k);
                    year_month ym(y, m);
                    year_month_day ymd = ym/d;
                    assert(ymd.year()  == y);
                    assert(ymd.month() == m);
                    assert(ymd.day()   == d);
                }
    }


    { // operator/(const year_month& ym, int d)
        ASSERT_NOEXCEPT (                         Feb2018/2);
        ASSERT_SAME_TYPE(year_month_day, decltype(Feb2018/2));

        static_assert((Feb2018/2).month() == February, "");
        static_assert((Feb2018/2).day()   == day{2},   "");

        for (int i = 1000; i < 1010; ++i)
            for (int j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 28; ++k)
                {
                    year y(i);
                    month m(j);
                    day d(k);
                    year_month ym(y, m);
                    year_month_day ymd = ym/k;
                    assert(ymd.year()  == y);
                    assert(ymd.month() == m);
                    assert(ymd.day()   == d);
                }
    }


    { // operator/(const year_month& ym, int d)
        ASSERT_NOEXCEPT (                         Feb2018/2);
        ASSERT_SAME_TYPE(year_month_day, decltype(Feb2018/2));

        static_assert((Feb2018/2).month() == February, "");
        static_assert((Feb2018/2).day()   == day{2},   "");

        for (int i = 1000; i < 1010; ++i)
            for (int j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 28; ++k)
                {
                    year y(i);
                    month m(j);
                    day d(k);
                    year_month ym(y, m);
                    year_month_day ymd = ym/k;
                    assert(ymd.year()  == y);
                    assert(ymd.month() == m);
                    assert(ymd.day()   == d);
                }
    }




    { // operator/(const year& y, const month_day& md) (and switched)
        ASSERT_NOEXCEPT (                         year{2018}/month_day{February, day{2}});
        ASSERT_SAME_TYPE(year_month_day, decltype(year{2018}/month_day{February, day{2}}));
        ASSERT_NOEXCEPT (                         month_day{February, day{2}}/year{2018});
        ASSERT_SAME_TYPE(year_month_day, decltype(month_day{February, day{2}}/year{2018}));

        static_assert((year{2018}/month_day{February, day{2}}).month() == February, "" );
        static_assert((year{2018}/month_day{February, day{2}}).day()   == day{2},   "" );
        static_assert((month_day{February, day{2}}/year{2018}).month() == February, "" );
        static_assert((month_day{February, day{2}}/year{2018}).day()   == day{2},   "" );

        for (int i = 1000; i < 1010; ++i)
            for (int j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 28; ++k)
                {
                    year y(i);
                    month m(j);
                    day d(k);
                    month_day md(m, d);
                    year_month_day ymd1 = y/md;
                    year_month_day ymd2 = md/y;
                    assert(ymd1.year()  == y);
                    assert(ymd2.year()  == y);
                    assert(ymd1.month() == m);
                    assert(ymd2.month() == m);
                    assert(ymd1.day()   == d);
                    assert(ymd2.day()   == d);
                    assert(ymd1 == ymd2);
                }
    }

    { // operator/(const month_day& md, int y) (and switched)
        ASSERT_NOEXCEPT (                         2018/month_day{February, day{2}});
        ASSERT_SAME_TYPE(year_month_day, decltype(2018/month_day{February, day{2}}));
        ASSERT_NOEXCEPT (                         month_day{February, day{2}}/2018);
        ASSERT_SAME_TYPE(year_month_day, decltype(month_day{February, day{2}}/2018));

        static_assert((2018/month_day{February, day{2}}).month() == February, "" );
        static_assert((2018/month_day{February, day{2}}).day()   == day{2},   "" );
        static_assert((month_day{February, day{2}}/2018).month() == February, "" );
        static_assert((month_day{February, day{2}}/2018).day()   == day{2},   "" );

        for (int i = 1000; i < 1010; ++i)
            for (int j = 1; j <= 12; ++j)
                for (unsigned k = 0; k <= 28; ++k)
                {
                    year y(i);
                    month m(j);
                    day d(k);
                    month_day md(m, d);
                    year_month_day ymd1 = i/md;
                    year_month_day ymd2 = md/i;
                    assert(ymd1.year()  == y);
                    assert(ymd2.year()  == y);
                    assert(ymd1.month() == m);
                    assert(ymd2.month() == m);
                    assert(ymd1.day()   == d);
                    assert(ymd2.day()   == d);
                    assert(ymd1 == ymd2);
                }
    }

}
