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

// constexpr chrono::month month() const noexcept;
//  Returns: m_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using month     = std::chrono::month;
    using month_day_last = std::chrono::month_day_last;

    ASSERT_NOEXCEPT(                 std::declval<const month_day_last>().month());
    ASSERT_SAME_TYPE(month, decltype(std::declval<const month_day_last>().month()));

    static_assert( month_day_last{month{}}.month() == month{}, "");

    for (unsigned i = 1; i <= 50; ++i)
    {
        month_day_last mdl(month{i});
        assert( static_cast<unsigned>(mdl.month()) == i);
    }
}
