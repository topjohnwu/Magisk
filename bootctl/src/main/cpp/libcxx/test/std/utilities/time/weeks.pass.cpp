//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <chrono>

// using weeks = duration<signed integer type of at least 22 bits, ratio_multiply<ratio<7>, days::period>>;

#include <chrono>
#include <type_traits>
#include <limits>

int main()
{
    typedef std::chrono::weeks D;
    typedef D::rep Rep;
    typedef D::period Period;
    static_assert(std::is_signed<Rep>::value, "");
    static_assert(std::is_integral<Rep>::value, "");
    static_assert(std::numeric_limits<Rep>::digits >= 22, "");
    static_assert(std::is_same_v<Period, std::ratio_multiply<std::ratio<7>, std::chrono::days::period>>, "");
}
