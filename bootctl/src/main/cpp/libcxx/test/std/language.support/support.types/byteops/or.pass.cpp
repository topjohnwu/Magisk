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

// constexpr byte operator|(byte l, byte r) noexcept;

int main () {
    constexpr std::byte b1{static_cast<std::byte>(1)};
    constexpr std::byte b2{static_cast<std::byte>(2)};
    constexpr std::byte b8{static_cast<std::byte>(8)};

    static_assert(noexcept(b1 | b2), "" );

    static_assert(std::to_integer<int>(b1 | b2) ==  3, "");
    static_assert(std::to_integer<int>(b1 | b8) ==  9, "");
    static_assert(std::to_integer<int>(b2 | b8) == 10, "");

    static_assert(std::to_integer<int>(b2 | b1) ==  3, "");
    static_assert(std::to_integer<int>(b8 | b1) ==  9, "");
    static_assert(std::to_integer<int>(b8 | b2) == 10, "");
}
