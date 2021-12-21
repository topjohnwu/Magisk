//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration_values::zero  // noexcept after C++17

#include <chrono>
#include <cassert>

#include "test_macros.h"
#include "../../rep.h"

int main()
{
    assert(std::chrono::duration_values<int>::zero() == 0);
    assert(std::chrono::duration_values<Rep>::zero() == 0);
#if TEST_STD_VER >= 11
    static_assert(std::chrono::duration_values<int>::zero() == 0, "");
    static_assert(std::chrono::duration_values<Rep>::zero() == 0, "");
#endif

    LIBCPP_ASSERT_NOEXCEPT(std::chrono::duration_values<int>::zero());
    LIBCPP_ASSERT_NOEXCEPT(std::chrono::duration_values<Rep>::zero());
#if TEST_STD_VER > 17
    ASSERT_NOEXCEPT(std::chrono::duration_values<int>::zero());
    ASSERT_NOEXCEPT(std::chrono::duration_values<Rep>::zero());
#endif
}
