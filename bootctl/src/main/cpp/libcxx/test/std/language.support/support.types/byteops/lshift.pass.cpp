//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cstddef>
#include <test_macros.h>

// UNSUPPORTED: c++98, c++03, c++11, c++14

// template <class IntegerType>
//    constexpr byte operator <<(byte b, IntegerType shift) noexcept;
// These functions shall not participate in overload resolution unless
//   is_integral_v<IntegerType> is true.

int main () {
    constexpr std::byte b1{static_cast<std::byte>(1)};
    constexpr std::byte b3{static_cast<std::byte>(3)};

    static_assert(noexcept(b3 << 2), "" );

    static_assert(std::to_integer<int>(b1 << 1) ==   2, "");
    static_assert(std::to_integer<int>(b1 << 2) ==   4, "");
    static_assert(std::to_integer<int>(b3 << 4) ==  48, "");
    static_assert(std::to_integer<int>(b3 << 6) == 192, "");
}
