//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// time_point

// static constexpr time_point min(); // noexcept after C++17

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::chrono::system_clock Clock;
    typedef std::chrono::milliseconds Duration;
    typedef std::chrono::time_point<Clock, Duration> TP;
    LIBCPP_ASSERT_NOEXCEPT(TP::max());
#if TEST_STD_VER > 17
    ASSERT_NOEXCEPT(       TP::max());
#endif
    assert(TP::min() == TP(Duration::min()));
}
