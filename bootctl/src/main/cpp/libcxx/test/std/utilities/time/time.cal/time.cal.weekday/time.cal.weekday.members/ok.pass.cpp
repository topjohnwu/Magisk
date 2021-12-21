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

//  constexpr bool ok() const noexcept;
//  Returns: wd_ <= 6

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using weekday = std::chrono::weekday;

    ASSERT_NOEXCEPT(                std::declval<const weekday>().ok());
    ASSERT_SAME_TYPE(bool, decltype(std::declval<const weekday>().ok()));

    static_assert( weekday{0}.ok(), "");
    static_assert( weekday{1}.ok(), "");
    static_assert(!weekday{7}.ok(), "");

    for (unsigned i = 0; i <= 6; ++i)
        assert(weekday{i}.ok());
    for (unsigned i = 7; i <= 255; ++i)
        assert(!weekday{i}.ok());
}
