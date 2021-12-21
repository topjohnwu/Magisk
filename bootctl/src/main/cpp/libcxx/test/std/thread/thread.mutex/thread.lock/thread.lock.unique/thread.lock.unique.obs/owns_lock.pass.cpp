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

// bool owns_lock() const;

#include <mutex>
#include <cassert>

std::mutex m;

int main()
{
    std::unique_lock<std::mutex> lk0;
    assert(lk0.owns_lock() == false);
    std::unique_lock<std::mutex> lk1(m);
    assert(lk1.owns_lock() == true);
    lk1.unlock();
    assert(lk1.owns_lock() == false);
}
