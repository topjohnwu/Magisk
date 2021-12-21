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

//   constexpr weekday_indexed operator[](unsigned index) const noexcept;
//   constexpr weekday_last    operator[](last_spec)      const noexcept;


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "../../euclidian.h"

int main()
{
    using weekday         = std::chrono::weekday;
    using weekday_last    = std::chrono::weekday_last;
    using weekday_indexed = std::chrono::weekday_indexed;

    constexpr weekday Sunday = std::chrono::Sunday;

    ASSERT_NOEXCEPT(                           std::declval<weekday>()[1U]);
    ASSERT_SAME_TYPE(weekday_indexed, decltype(std::declval<weekday>()[1U]));

    ASSERT_NOEXCEPT(                           std::declval<weekday>()[std::chrono::last]);
    ASSERT_SAME_TYPE(weekday_last,    decltype(std::declval<weekday>()[std::chrono::last]));

    static_assert(Sunday[2].weekday() == Sunday, "");
    static_assert(Sunday[2].index  () == 2, "");

    for (unsigned i = 0; i <= 6; ++i)
    {
        weekday wd(i);
        weekday_last wdl = wd[std::chrono::last];
        assert(wdl.weekday() == wd);
        assert(wdl.ok());
    }

    for (unsigned i = 0; i <= 6; ++i)
        for (unsigned j = 1; j <= 5; ++j)
    {
        weekday wd(i);
        weekday_indexed wdi = wd[j];
        assert(wdi.weekday() == wd);
        assert(wdi.index() == j);
        assert(wdi.ok());
    }
}
