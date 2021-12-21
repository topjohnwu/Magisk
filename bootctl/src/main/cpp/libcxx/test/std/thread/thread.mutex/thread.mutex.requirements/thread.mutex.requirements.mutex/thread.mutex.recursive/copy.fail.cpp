//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <mutex>

// class recursive_mutex;

// recursive_mutex(const recursive_mutex&) = delete;

#include <mutex>

int main()
{
    std::recursive_mutex m0;
    std::recursive_mutex m1(m0);
}
