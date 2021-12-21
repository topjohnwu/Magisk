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

// static time_point from_time_t(time_t t);

#include <chrono>
#include <ctime>

int main()
{
    typedef std::chrono::system_clock C;
    C::time_point t1 = C::from_time_t(C::to_time_t(C::now()));
    ((void)t1);
}
