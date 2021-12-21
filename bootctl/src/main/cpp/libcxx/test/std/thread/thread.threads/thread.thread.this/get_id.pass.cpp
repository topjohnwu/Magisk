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

// <thread>

// thread::id this_thread::get_id();

#include <thread>
#include <cassert>

int main()
{
    std::thread::id id = std::this_thread::get_id();
    assert(id != std::thread::id());
}
