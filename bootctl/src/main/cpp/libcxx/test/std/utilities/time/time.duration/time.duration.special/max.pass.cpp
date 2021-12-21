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

// static constexpr duration max(); // noexcept after C++17

#include <chrono>
#include <limits>
#include <cassert>

#include "test_macros.h"
#include "../../rep.h"

template <class D>
void test()
{
    LIBCPP_ASSERT_NOEXCEPT(std::chrono::duration_values<typename D::rep>::max());
#if TEST_STD_VER > 17
    ASSERT_NOEXCEPT(       std::chrono::duration_values<typename D::rep>::max());
#endif
    {
    typedef typename D::rep Rep;
    Rep max_rep = std::chrono::duration_values<Rep>::max();
    assert(D::max().count() == max_rep);
    }
#if TEST_STD_VER >= 11
    {
    typedef typename D::rep Rep;
    constexpr Rep max_rep = std::chrono::duration_values<Rep>::max();
    static_assert(D::max().count() == max_rep, "");
    }
#endif
}

int main()
{
    test<std::chrono::duration<int> >();
    test<std::chrono::duration<Rep> >();
}
