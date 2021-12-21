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

// #define ATOMIC_VAR_INIT(value)

#include <atomic>
#include <type_traits>
#include <cassert>

int main()
{
    std::atomic<int> v = ATOMIC_VAR_INIT(5);
    assert(v == 5);
}
