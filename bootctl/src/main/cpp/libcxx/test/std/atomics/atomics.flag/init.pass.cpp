//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// XFAIL: c++98, c++03

// <atomic>

// struct atomic_flag

// atomic_flag() = ATOMIC_FLAG_INIT;

#include <atomic>
#include <cassert>

int main()
{
    std::atomic_flag f = ATOMIC_FLAG_INIT;
    assert(f.test_and_set() == 0);
}
