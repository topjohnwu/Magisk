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
// class year;

//  constexpr year& operator--() noexcept;
//  constexpr year operator--(int) noexcept;


#include <chrono>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

template <typename Y>
constexpr bool testConstexpr()
{
    Y y1{10};
    if (static_cast<int>(--y1) != 9) return false;
    if (static_cast<int>(y1--) != 9) return false;
    if (static_cast<int>(y1)   != 8) return false;
    return true;
}

int main()
{
    using year = std::chrono::year;
    ASSERT_NOEXCEPT(--(std::declval<year&>())  );
    ASSERT_NOEXCEPT(  (std::declval<year&>())--);

    ASSERT_SAME_TYPE(year , decltype(  std::declval<year&>()--));
    ASSERT_SAME_TYPE(year&, decltype(--std::declval<year&>()  ));

    static_assert(testConstexpr<year>(), "");

    for (int i = 11000; i <= 11020; ++i)
    {
        year year(i);
        assert(static_cast<int>(--year) == i - 1);
        assert(static_cast<int>(year--) == i - 1);
        assert(static_cast<int>(year)   == i - 2);
    }
}
