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

// class shared_timed_mutex;

// shared_timed_mutex& operator=(const shared_timed_mutex&) = delete;

#include <shared_mutex>

int main()
{
    std::shared_timed_mutex m0;
    std::shared_timed_mutex m1;
    m1 = m0;
}
