//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// time_point

// Duration shall be an instance of duration.

#include <chrono>

int main()
{
    typedef std::chrono::time_point<std::chrono::system_clock, int> T;
    T t;
}
