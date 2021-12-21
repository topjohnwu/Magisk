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

// constexpr byte& operator &=(byte l, byte r) noexcept;


constexpr std::byte test(std::byte b1, std::byte b2) {
    std::byte bret = b1;
    return bret &= b2;
    }


int main () {
    std::byte b;  // not constexpr, just used in noexcept check
    constexpr std::byte b1{static_cast<std::byte>(1)};
    constexpr std::byte b8{static_cast<std::byte>(8)};
    constexpr std::byte b9{static_cast<std::byte>(9)};

    static_assert(noexcept(b &= b), "" );

    static_assert(std::to_integer<int>(test(b1, b8)) == 0, "");
    static_assert(std::to_integer<int>(test(b1, b9)) == 1, "");
    static_assert(std::to_integer<int>(test(b8, b9)) == 8, "");

    static_assert(std::to_integer<int>(test(b8, b1)) == 0, "");
    static_assert(std::to_integer<int>(test(b9, b1)) == 1, "");
    static_assert(std::to_integer<int>(test(b9, b8)) == 8, "");
}
