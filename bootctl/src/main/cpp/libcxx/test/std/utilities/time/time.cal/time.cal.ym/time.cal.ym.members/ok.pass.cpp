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

// constexpr bool ok() const noexcept;
//  Returns: m_.ok() && y_.ok().

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using month      = std::chrono::month;
    using year       = std::chrono::year;
    using year_month = std::chrono::year_month;

    constexpr month January = std::chrono::January;

    ASSERT_NOEXCEPT(                std::declval<const year_month>().ok());
    ASSERT_SAME_TYPE(bool, decltype(std::declval<const year_month>().ok()));

    static_assert(!year_month{year{-32768}, January}.ok(), ""); // Bad year
    static_assert(!year_month{year{2019},   month{}}.ok(), ""); // Bad month
    static_assert( year_month{year{2019},   January}.ok(), ""); // Both OK

    for (unsigned i = 0; i <= 50; ++i)
    {
        year_month ym{year{2019}, month{i}};
        assert( ym.ok() == month{i}.ok());
    }

    const int ymax = static_cast<int>(year::max());
    for (int i = ymax - 100; i <= ymax + 100; ++i)
    {
        year_month ym{year{i}, January};
        assert( ym.ok() == year{i}.ok());
    }
}
