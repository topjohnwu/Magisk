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

// <mutex>

// template <class Mutex> class unique_lock;

// unique_lock();

#include <mutex>
#include <cassert>

int main()
{
    std::unique_lock<std::mutex> ul;
    assert(!ul.owns_lock());
    assert(ul.mutex() == nullptr);
}
