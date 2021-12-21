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
// class weekday;

// constexpr weekday operator-(const weekday& x, const days& y) noexcept;
//   Returns: x + -y.
//
// constexpr days operator-(const weekday& x, const weekday& y) noexcept;
// Returns: If x.ok() == true and y.ok() == true, returns a value d in the range
//    [days{0}, days{6}] satisfying y + d == x.
// Otherwise the value returned is unspecified.
// [Example: Sunday - Monday == days{6}. —end example]


extern "C" int printf(const char *, ...);

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "../../euclidian.h"

template <typename WD, typename Ds>
constexpr bool testConstexpr()
{
    {
    WD wd{5};
    Ds offset{3};
    if (wd - offset != WD{2}) return false;
    if (wd - WD{2} != offset) return false;
    }

//  Check the example
    if (WD{0} - WD{1} != Ds{6}) return false;
    return true;
}

int main()
{
    using weekday  = std::chrono::weekday;
    using days     = std::chrono::days;

    ASSERT_NOEXCEPT(                   std::declval<weekday>() - std::declval<days>());
    ASSERT_SAME_TYPE(weekday, decltype(std::declval<weekday>() - std::declval<days>()));

    ASSERT_NOEXCEPT(                   std::declval<weekday>() - std::declval<weekday>());
    ASSERT_SAME_TYPE(days,    decltype(std::declval<weekday>() - std::declval<weekday>()));

    static_assert(testConstexpr<weekday, days>(), "");

    for (unsigned i = 0; i <= 6; ++i)
        for (unsigned j = 0; j <= 6; ++j)
        {
            weekday wd = weekday{i} - days{j};
            assert(wd + days{j} == weekday{i});
            assert((static_cast<unsigned>(wd) == euclidian_subtraction<unsigned, 0, 6>(i, j)));
        }

    for (unsigned i = 0; i <= 6; ++i)
        for (unsigned j = 0; j <= 6; ++j)
        {
            days d = weekday{j} - weekday{i};
            assert(weekday{i} + d == weekday{j});
        }

}
