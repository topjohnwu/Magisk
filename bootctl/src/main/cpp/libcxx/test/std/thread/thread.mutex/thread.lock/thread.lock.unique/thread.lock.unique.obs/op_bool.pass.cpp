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

// explicit operator bool() const;

#include <mutex>
#include <cassert>

std::mutex m;

int main()
{
    std::unique_lock<std::mutex> lk0;
    assert(static_cast<bool>(lk0) == false);
    std::unique_lock<std::mutex> lk1(m);
    assert(static_cast<bool>(lk1) == true);
    lk1.unlock();
    assert(static_cast<bool>(lk1) == false);
}
