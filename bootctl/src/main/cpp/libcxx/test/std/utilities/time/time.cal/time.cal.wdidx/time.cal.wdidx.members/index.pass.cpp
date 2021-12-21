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
// class weekday_indexed;

// constexpr unsigned index() const noexcept;
//  Returns: index_

#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    using weekday         = std::chrono::weekday;
    using weekday_indexed = std::chrono::weekday_indexed;

    ASSERT_NOEXCEPT(                    std::declval<const weekday_indexed>().index());
    ASSERT_SAME_TYPE(unsigned, decltype(std::declval<const weekday_indexed>().index()));

    static_assert( weekday_indexed{}.index() == 0, "");

    for (unsigned i = 1; i <= 5; ++i)
    {
        weekday_indexed wdi(weekday{2}, i);
        assert( static_cast<unsigned>(wdi.index()) == i);
    }
}
