//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// typedef duration<signed integral type of at least 23 bits, ratio<3600>> hours;

#include <chrono>
#include <type_traits>
#include <limits>

int main()
{
    typedef std::chrono::hours D;
    typedef D::rep Rep;
    typedef D::period Period;
    static_assert(std::is_signed<Rep>::value, "");
    static_assert(std::is_integral<Rep>::value, "");
    static_assert(std::numeric_limits<Rep>::digits >= 22, "");
    static_assert((std::is_same<Period, std::ratio<3600> >::value), "");
}
