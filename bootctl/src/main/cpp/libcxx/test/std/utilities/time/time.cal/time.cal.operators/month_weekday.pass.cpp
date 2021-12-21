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
// class month_weekday;

// constexpr month_weekday
//   operator/(const month& m, const weekday_indexed& wdi) noexcept;
// Returns: {m, wdi}.
//
// constexpr month_weekday
//   operator/(int m, const weekday_indexed& wdi) noexcept;
// Returns: month(m) / wdi.
//
// constexpr month_weekday
//   operator/(const weekday_indexed& wdi, const month& m) noexcept;
// Returns: m / wdi. constexpr month_weekday
//
// constexpr month_weekday
//   operator/(const weekday_indexed& wdi, int m) noexcept;
// Returns: month(m) / wdi.


//
// [Example:
// constexpr auto mwd = February/Tuesday[3]; // mwd is the third Tuesday of February of an as yet unspecified year
//      static_assert(mwd.month() == February);
//      static_assert(mwd.weekday_indexed() == Tuesday[3]);
// —end example]




#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using month_weekday   = std::chrono::month_weekday;
    using month           = std::chrono::month;
    using weekday         = std::chrono::weekday;
    using weekday_indexed = std::chrono::weekday_indexed;

    constexpr weekday Tuesday = std::chrono::Tuesday;
    constexpr month February = std::chrono::February;

    { // operator/(const month& m, const weekday_indexed& wdi) (and switched)
        ASSERT_NOEXCEPT (February/Tuesday[2]);
        ASSERT_SAME_TYPE(month_weekday, decltype(February/Tuesday[2]));
        ASSERT_NOEXCEPT (Tuesday[2]/February);
        ASSERT_SAME_TYPE(month_weekday, decltype(Tuesday[2]/February));

    //  Run the example
        {
        constexpr month_weekday wdi = February/Tuesday[3];
        static_assert(wdi.month()           == February,   "");
        static_assert(wdi.weekday_indexed() == Tuesday[3], "");
        }

        for (int i = 1; i <= 12; ++i)
            for (unsigned j = 0; j <= 6; ++j)
                for (unsigned k = 1; k <= 5; ++k)
                {
                    month m(i);
                    weekday_indexed wdi = weekday{j}[k];
                    month_weekday mwd1 = m/wdi;
                    month_weekday mwd2 = wdi/m;
                    assert(mwd1.month() == m);
                    assert(mwd1.weekday_indexed() == wdi);
                    assert(mwd2.month() == m);
                    assert(mwd2.weekday_indexed() == wdi);
                    assert(mwd1 == mwd2);
                }
    }


    { // operator/(int m, const weekday_indexed& wdi) (and switched)
        ASSERT_NOEXCEPT (2/Tuesday[2]);
        ASSERT_SAME_TYPE(month_weekday, decltype(2/Tuesday[2]));
        ASSERT_NOEXCEPT (Tuesday[2]/2);
        ASSERT_SAME_TYPE(month_weekday, decltype(Tuesday[2]/2));

    //  Run the example
        {
        constexpr month_weekday wdi = 2/Tuesday[3];
        static_assert(wdi.month()           == February,   "");
        static_assert(wdi.weekday_indexed() == Tuesday[3], "");
        }

        for (int i = 1; i <= 12; ++i)
            for (unsigned j = 0; j <= 6; ++j)
                for (unsigned k = 1; k <= 5; ++k)
                {
                    weekday_indexed wdi = weekday{j}[k];
                    month_weekday mwd1 = i/wdi;
                    month_weekday mwd2 = wdi/i;
                    assert(mwd1.month() == month(i));
                    assert(mwd1.weekday_indexed() == wdi);
                    assert(mwd2.month() == month(i));
                    assert(mwd2.weekday_indexed() == wdi);
                    assert(mwd1 == mwd2);
                }
    }
}
