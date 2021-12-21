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

// constexpr byte operator~(byte b) noexcept;

int main () {
    constexpr std::byte b1{static_cast<std::byte>(1)};
    constexpr std::byte b2{static_cast<std::byte>(2)};
    constexpr std::byte b8{static_cast<std::byte>(8)};

    static_assert(noexcept(~b1), "" );

    static_assert(std::to_integer<int>(~b1) == 254, "");
    static_assert(std::to_integer<int>(~b2) == 253, "");
    static_assert(std::to_integer<int>(~b8) == 247, "");
}
