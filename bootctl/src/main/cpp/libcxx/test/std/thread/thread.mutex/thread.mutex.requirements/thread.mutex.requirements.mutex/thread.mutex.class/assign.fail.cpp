//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <mutex>

// class mutex;

// mutex& operator=(const mutex&) = delete;

#include <mutex>

int main()
{
    std::mutex m0;
    std::mutex m1;
    m1 = m0;
}
