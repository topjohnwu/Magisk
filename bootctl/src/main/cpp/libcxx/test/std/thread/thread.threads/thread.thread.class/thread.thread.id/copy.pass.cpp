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

// class thread::id

// id(const id&) = default;

#include <thread>
#include <cassert>

int main()
{
    std::thread::id id0;
    std::thread::id id1 = id0;
    assert(id1 == id0);
}
