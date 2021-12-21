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
// class weekday_last;

//  constexpr chrono::weekday weekday() const noexcept;
//  Returns: wd_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using weekday      = std::chrono::weekday;
    using weekday_last = std::chrono::weekday_last;

    ASSERT_NOEXCEPT(                   std::declval<const weekday_last>().weekday());
    ASSERT_SAME_TYPE(weekday, decltype(std::declval<const weekday_last>().weekday()));

    for (unsigned i = 0; i <= 255; ++i)
        assert(weekday_last{weekday{i}}.weekday() == weekday{i});
}
