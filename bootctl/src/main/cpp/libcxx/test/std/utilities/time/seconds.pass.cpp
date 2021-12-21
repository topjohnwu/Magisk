//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// typedef duration<signed integral type of at least 35 bits > seconds;

#include <chrono>
#include <type_traits>
#include <limits>

int main()
{
    typedef std::chrono::seconds D;
    typedef D::rep Rep;
    typedef D::period Period;
    static_assert(std::is_signed<Rep>::value, "");
    static_assert(std::is_integral<Rep>::value, "");
    static_assert(std::numeric_limits<Rep>::digits >= 34, "");
    static_assert((std::is_same<Period, std::ratio<1> >::value), "");
}
