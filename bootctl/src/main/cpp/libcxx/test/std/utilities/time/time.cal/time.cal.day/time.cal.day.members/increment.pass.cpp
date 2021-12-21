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
// class day;

//  constexpr day& operator++() noexcept;
//  constexpr day operator++(int) noexcept;


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename D>
constexpr bool testConstexpr()
{
    D d1{1};
    if (static_cast<unsigned>(++d1) != 2) return false;
    if (static_cast<unsigned>(d1++) != 2) return false;
    if (static_cast<unsigned>(d1)   != 3) return false;
    return true;
}

int main()
{
    using day = std::chrono::day;
    ASSERT_NOEXCEPT(++(std::declval<day&>())  );
    ASSERT_NOEXCEPT(  (std::declval<day&>())++);

    ASSERT_SAME_TYPE(day , decltype(  std::declval<day&>()++));
    ASSERT_SAME_TYPE(day&, decltype(++std::declval<day&>()  ));

    static_assert(testConstexpr<day>(), "");

    for (unsigned i = 10; i <= 20; ++i)
    {
        day day(i);
        assert(static_cast<unsigned>(++day) == i + 1);
        assert(static_cast<unsigned>(day++) == i + 1);
        assert(static_cast<unsigned>(day)   == i + 2);
    }
}
