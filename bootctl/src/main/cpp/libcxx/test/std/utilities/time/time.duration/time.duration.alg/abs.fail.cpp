//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14
// <chrono>

// ceil

// template <class Rep, class Period>
//   constexpr duration<Rep, Period> abs(duration<Rep, Period> d)

// This function shall not participate in overload resolution unless numeric_limits<Rep>::is_signed is true.

#include <chrono>

typedef std::chrono::duration<unsigned> unsigned_secs;

int main()
{
    std::chrono::abs(unsigned_secs(0));
}
