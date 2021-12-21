//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// system_clock

// time_t to_time_t(const time_point& t);

#include <chrono>
#include <ctime>

int main()
{
    typedef std::chrono::system_clock C;
    std::time_t t1 = C::to_time_t(C::now());
    ((void)t1);
}
