//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration

// static constexpr duration zero(); // noexcept after C++17

#include <chrono>
#include <cassert>

#include "test_macros.h"
#include "../../rep.h"

template <class D>
void test()
{
    LIBCPP_ASSERT_NOEXCEPT(std::chrono::duration_values<typename D::rep>::zero());
#if TEST_STD_VER > 17
    ASSERT_NOEXCEPT(       std::chrono::duration_values<typename D::rep>::zero());
#endif
    {
    typedef typename D::rep Rep;
    Rep zero_rep = std::chrono::duration_values<Rep>::zero();
    assert(D::zero().count() == zero_rep);
    }
#if TEST_STD_VER >= 11
    {
    typedef typename D::rep Rep;
    constexpr Rep zero_rep = std::chrono::duration_values<Rep>::zero();
    static_assert(D::zero().count() == zero_rep, "");
    }
#endif
}

int main()
{
    test<std::chrono::duration<int> >();
    test<std::chrono::duration<Rep> >();
}
