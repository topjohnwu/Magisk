//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <mutex>

// class timed_mutex;

// timed_mutex(const timed_mutex&) = delete;

#include <mutex>

int main()
{
    std::timed_mutex m0;
    std::timed_mutex m1(m0);
}
