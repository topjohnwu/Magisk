//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// TODO: Remove this when filesystem gets integrated into the dylib
// REQUIRES: c++filesystem

// <chrono>

// file_clock

// static time_point now() noexcept;

#include <chrono>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::chrono::file_clock C;
    ASSERT_NOEXCEPT(C::now());

    C::time_point t1 = C::now();
    assert(t1.time_since_epoch().count() != 0);
    assert(C::time_point::min() < t1);
    assert(C::time_point::max() > t1);
}
