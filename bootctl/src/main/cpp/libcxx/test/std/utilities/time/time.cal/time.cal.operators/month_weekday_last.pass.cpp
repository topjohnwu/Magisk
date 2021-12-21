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
// class month_weekday_last;

// constexpr month_weekday_last
//   operator/(const month& m, const weekday_last& wdl) noexcept;
// Returns: {m, wdl}.
//
// constexpr month_weekday_last
//   operator/(int m, const weekday_last& wdl) noexcept;
// Returns: month(m) / wdl.
//
// constexpr month_weekday_last
//   operator/(const weekday_last& wdl, const month& m) noexcept;
// Returns: m / wdl.
//
// constexpr month_weekday_last
//   operator/(const weekday_last& wdl, int m) noexcept;
// Returns: month(m) / wdl.




#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using month_weekday      = std::chrono::month_weekday;
    using month              = std::chrono::month;
    using weekday            = std::chrono::weekday;
    using weekday_last       = std::chrono::weekday_last;
    using month_weekday_last = std::chrono::month_weekday_last;

    constexpr weekday Tuesday = std::chrono::Tuesday;
    constexpr month February = std::chrono::February;
    constexpr std::chrono::last_spec last = std::chrono::last;

    { // operator/(const month& m, const weekday_last& wdi) (and switched)
        ASSERT_NOEXCEPT (February/Tuesday[last]);
        ASSERT_SAME_TYPE(month_weekday_last, decltype(February/Tuesday[last]));
        ASSERT_NOEXCEPT (Tuesday[last]/February);
        ASSERT_SAME_TYPE(month_weekday_last, decltype(Tuesday[last]/February));

    //  Run the example
        {
        constexpr month_weekday_last wdi = February/Tuesday[last];
        static_assert(wdi.month()        == February,      "");
        static_assert(wdi.weekday_last() == Tuesday[last], "");
        }

        for (int i = 1; i <= 12; ++i)
            for (unsigned j = 0; j <= 6; ++j)
            {
                month m(i);
                weekday_last wdi = weekday{j}[last];
                month_weekday_last mwd1 = m/wdi;
                month_weekday_last mwd2 = wdi/m;
                assert(mwd1.month() == m);
                assert(mwd1.weekday_last() == wdi);
                assert(mwd2.month() == m);
                assert(mwd2.weekday_last() == wdi);
                assert(mwd1 == mwd2);
            }
    }


    { // operator/(int m, const weekday_last& wdi) (and switched)
        ASSERT_NOEXCEPT (2/Tuesday[2]);
        ASSERT_SAME_TYPE(month_weekday_last, decltype(2/Tuesday[last]));
        ASSERT_NOEXCEPT (Tuesday[2]/2);
        ASSERT_SAME_TYPE(month_weekday_last, decltype(Tuesday[last]/2));

    //  Run the example
        {
        constexpr month_weekday wdi = 2/Tuesday[3];
        static_assert(wdi.month()           == February,   "");
        static_assert(wdi.weekday_indexed() == Tuesday[3], "");
        }

        for (int i = 1; i <= 12; ++i)
            for (unsigned j = 0; j <= 6; ++j)
            {
                weekday_last wdi = weekday{j}[last];
                month_weekday_last mwd1 = i/wdi;
                month_weekday_last mwd2 = wdi/i;
                assert(mwd1.month() == month(i));
                assert(mwd1.weekday_last() == wdi);
                assert(mwd2.month() == month(i));
                assert(mwd2.weekday_last() == wdi);
                assert(mwd1 == mwd2);
            }
    }
}
