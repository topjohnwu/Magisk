//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <shared_mutex>

// template <class Mutex> class shared_lock;

// shared_lock& operator=(shared_lock const&) = delete;

#include <shared_mutex>

std::shared_timed_mutex m0;
std::shared_timed_mutex m1;

int main()
{
    std::shared_lock<std::shared_timed_mutex> lk0(m0);
    std::shared_lock<std::shared_timed_mutex> lk1(m1);
    lk1 = lk0;
}
