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

// constexpr year_month operator/(const year& y, const month& m) noexcept;
//   Returns: {y, m}.
//
// constexpr year_month operator/(const year& y, int m) noexcept;
//   Returns: y / month(m).



#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using month      = std::chrono::month;
    using year       = std::chrono::year;
    using year_month = std::chrono::year_month;

    constexpr month February = std::chrono::February;

    { // operator/(const year& y, const month& m)
        ASSERT_NOEXCEPT (                     year{2018}/February);
        ASSERT_SAME_TYPE(year_month, decltype(year{2018}/February));

        static_assert((year{2018}/February).year()  == year{2018}, "");
        static_assert((year{2018}/February).month() == month{2},   "");
        for (int i = 1000; i <= 1030; ++i)
            for (unsigned j = 1; j <= 12; ++j)
            {
                year_month ym = year{i}/month{j};
                assert(static_cast<int>(ym.year())       == i);
                assert(static_cast<unsigned>(ym.month()) == j);
            }
    }


    { // operator/(const year& y, const int m)
        ASSERT_NOEXCEPT (                     year{2018}/4);
        ASSERT_SAME_TYPE(year_month, decltype(year{2018}/4));

        static_assert((year{2018}/2).year()  == year{2018}, "");
        static_assert((year{2018}/2).month() == month{2},   "");

        for (int i = 1000; i <= 1030; ++i)
            for (unsigned j = 1; j <= 12; ++j)
            {
                year_month ym = year{i}/j;
                assert(static_cast<int>(ym.year())       == i);
                assert(static_cast<unsigned>(ym.month()) == j);
            }
    }
}
