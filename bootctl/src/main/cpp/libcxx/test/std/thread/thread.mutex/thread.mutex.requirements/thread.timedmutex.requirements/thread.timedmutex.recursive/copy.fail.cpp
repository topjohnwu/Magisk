//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <mutex>

// class recursive_timed_mutex;

// recursive_timed_mutex(const recursive_timed_mutex&) = delete;

#include <mutex>

int main()
{
    std::recursive_timed_mutex m0;
    std::recursive_timed_mutex m1(m0);
}
