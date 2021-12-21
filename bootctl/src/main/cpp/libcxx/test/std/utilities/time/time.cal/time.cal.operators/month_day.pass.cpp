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
// class month_day;

// constexpr month_day
//   operator/(const month& m, const day& d) noexcept;
// Returns: {m, d}.
//
// constexpr month_day
//   operator/(const day& d, const month& m) noexcept;
// Returns: m / d.

// constexpr month_day
//   operator/(const month& m, int d) noexcept;
// Returns: m / day(d).
//
// constexpr month_day
//   operator/(int m, const day& d) noexcept;
// Returns: month(m) / d.
//
// constexpr month_day
//   operator/(const day& d, int m) noexcept;
// Returns: month(m) / d.


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using month_day = std::chrono::month_day;
    using month     = std::chrono::month;
    using day       = std::chrono::day;

    constexpr month February = std::chrono::February;

    { // operator/(const month& m, const day& d) (and switched)
        ASSERT_NOEXCEPT (                    February/day{1});
        ASSERT_SAME_TYPE(month_day, decltype(February/day{1}));
        ASSERT_NOEXCEPT (                    day{1}/February);
        ASSERT_SAME_TYPE(month_day, decltype(day{1}/February));

        for (int i = 1; i <= 12; ++i)
            for (unsigned j = 0; j <= 30; ++j)
            {
                month m(i);
                day   d{j};
                month_day md1 = m/d;
                month_day md2 = d/m;
                assert(md1.month() == m);
                assert(md1.day()   == d);
                assert(md2.month() == m);
                assert(md2.day()   == d);
                assert(md1 == md2);
            }
    }


    { // operator/(const month& m, int d) (NOT switched)
        ASSERT_NOEXCEPT (                    February/2);
        ASSERT_SAME_TYPE(month_day, decltype(February/2));

        for (int i = 1; i <= 12; ++i)
            for (unsigned j = 0; j <= 30; ++j)
            {
                month m(i);
                day   d(j);
                month_day md1 = m/j;
                assert(md1.month() == m);
                assert(md1.day()   == d);
            }
    }


    { // operator/(const day& d, int m) (and switched)
        ASSERT_NOEXCEPT (                    day{2}/2);
        ASSERT_SAME_TYPE(month_day, decltype(day{2}/2));
        ASSERT_NOEXCEPT (                    2/day{2});
        ASSERT_SAME_TYPE(month_day, decltype(2/day{2}));

        for (int i = 1; i <= 12; ++i)
            for (unsigned j = 0; j <= 30; ++j)
            {
                month m(i);
                day   d(j);
                month_day md1 = d/i;
                month_day md2 = i/d;
                assert(md1.month() == m);
                assert(md1.day()   == d);
                assert(md2.month() == m);
                assert(md2.day()   == d);
                assert(md1 == md2);
            }
    }
}
