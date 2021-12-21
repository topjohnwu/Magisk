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
// class month_day_last;

// constexpr month_day_last
//   operator/(const month& m, last_spec) noexcept;
// Returns: month_day_last{m}.
//
// constexpr month_day_last
//   operator/(int m, last_spec) noexcept;
// Returns: month(m) / last.
//
// constexpr month_day_last
//   operator/(last_spec, const month& m) noexcept;
// Returns: m / last.
//
// constexpr month_day_last
//   operator/(last_spec, int m) noexcept;
// Returns: month(m) / last.
//
//
// [Note: A month_day_last object can be constructed using the expression m/last or last/m,
//     where m is an expression of type month. — end note]
// [Example:
//     constexpr auto mdl = February/last; // mdl is the last day of February of an as yet unspecified year
//     static_assert(mdl.month() == February);
// --end example]






#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_comparisons.h"

int main()
{
    using month          = std::chrono::month;
    using month_day_last = std::chrono::month_day_last;

    constexpr month February = std::chrono::February;
    constexpr std::chrono::last_spec last = std::chrono::last;

    ASSERT_SAME_TYPE(month_day_last, decltype(last/February));
    ASSERT_SAME_TYPE(month_day_last, decltype(February/last));

//  Run the example
    {
    constexpr auto mdl = February/std::chrono::last;
    static_assert(mdl.month() == February, "");
    }

    { // operator/(const month& m, last_spec) and switched
        ASSERT_NOEXCEPT (                         last/February);
        ASSERT_SAME_TYPE(month_day_last, decltype(last/February));
        ASSERT_NOEXCEPT (                         February/last);
        ASSERT_SAME_TYPE(month_day_last, decltype(February/last));

        static_assert((last/February).month() == February, "");
        static_assert((February/last).month() == February, "");

        for (unsigned i = 1; i < 12; ++i)
        {
            month m{i};
            month_day_last mdl1 = last/m;
            month_day_last mdl2 = m/last;
            assert(mdl1.month() == m);
            assert(mdl2.month() == m);
            assert(mdl1 == mdl2);
        }
    }

    { // operator/(int, last_spec) and switched
        ASSERT_NOEXCEPT (                         last/2);
        ASSERT_SAME_TYPE(month_day_last, decltype(last/2));
        ASSERT_NOEXCEPT (                         2/last);
        ASSERT_SAME_TYPE(month_day_last, decltype(2/last));

        static_assert((last/2).month() == February, "");
        static_assert((2/last).month() == February, "");

        for (unsigned i = 1; i < 12; ++i)
        {
            month m{i};
            month_day_last mdl1 = last/i;
            month_day_last mdl2 = i/last;
            assert(mdl1.month() == m);
            assert(mdl2.month() == m);
            assert(mdl1 == mdl2);
        }
    }

}
