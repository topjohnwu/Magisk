//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-monotonic-clock

// <chrono>

// steady_clock

// static time_point now();

#include <chrono>
#include <cassert>

int main()
{
    typedef std::chrono::steady_clock C;
    C::time_point t1 = C::now();
    C::time_point t2 = C::now();
    assert(t2 >= t1);
}
