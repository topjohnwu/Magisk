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
// UNSUPPORTED: c++98, c++03, c++11

// <shared_mutex>

// template <class Mutex> class shared_lock;

// explicit operator bool() const noexcept;

#include <shared_mutex>
#include <cassert>

std::shared_timed_mutex m;

int main()
{
    std::shared_lock<std::shared_timed_mutex> lk0;
    assert(static_cast<bool>(lk0) == false);
    std::shared_lock<std::shared_timed_mutex> lk1(m);
    assert(static_cast<bool>(lk1) == true);
    lk1.unlock();
    assert(static_cast<bool>(lk1) == false);
    static_assert(noexcept(static_cast<bool>(lk0)), "explicit operator bool() must be noexcept");
}
