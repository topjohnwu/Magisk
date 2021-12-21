//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration_values::max  // noexcept after C++17

#include <chrono>
#include <limits>
#include <cassert>

#include "test_macros.h"
#include "../../rep.h"

int main()
{
    assert(std::chrono::duration_values<int>::max() ==
           std::numeric_limits<int>::max());
    assert(std::chrono::duration_values<double>::max() ==
           std::numeric_limits<double>::max());
    assert(std::chrono::duration_values<Rep>::max() ==
           std::numeric_limits<Rep>::max());
#if TEST_STD_VER >= 11
    static_assert(std::chrono::duration_values<int>::max() ==
           std::numeric_limits<int>::max(), "");
    static_assert(std::chrono::duration_values<double>::max() ==
           std::numeric_limits<double>::max(), "");
    static_assert(std::chrono::duration_values<Rep>::max() ==
           std::numeric_limits<Rep>::max(), "");
#endif

    LIBCPP_ASSERT_NOEXCEPT(std::chrono::duration_values<int>::max());
    LIBCPP_ASSERT_NOEXCEPT(std::chrono::duration_values<double>::max());
    LIBCPP_ASSERT_NOEXCEPT(std::chrono::duration_values<Rep>::max());
#if TEST_STD_VER > 17
    ASSERT_NOEXCEPT(std::chrono::duration_values<int>::max());
    ASSERT_NOEXCEPT(std::chrono::duration_values<double>::max());
    ASSERT_NOEXCEPT(std::chrono::duration_values<Rep>::max());
#endif
}
