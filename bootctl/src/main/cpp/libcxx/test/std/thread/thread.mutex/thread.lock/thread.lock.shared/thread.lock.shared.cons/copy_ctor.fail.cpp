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

// shared_lock(shared_lock const&) = delete;

#include <shared_mutex>

std::shared_timed_mutex m;

int main()
{
    std::shared_lock<std::shared_timed_mutex> lk0(m);
    std::shared_lock<std::shared_timed_mutex> lk = lk0;
}
