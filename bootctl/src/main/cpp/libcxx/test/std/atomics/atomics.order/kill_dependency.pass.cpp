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

// <atomic>

// template <class T> T kill_dependency(T y);

#include <atomic>
#include <cassert>

int main()
{
    assert(std::kill_dependency(5) == 5);
    assert(std::kill_dependency(-5.5) == -5.5);
}
