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
//   constexpr byte& operator<<=(byte& b, IntegerType shift) noexcept;
// This function shall not participate in overload resolution unless
//   is_integral_v<IntegerType> is true.


constexpr std::byte test(std::byte b) {
    return b <<= 2;
    }


int main () {
    std::byte b;  // not constexpr, just used in noexcept check
    constexpr std::byte b2{static_cast<std::byte>(2)};
    constexpr std::byte b3{static_cast<std::byte>(3)};

    static_assert(noexcept(b <<= 2), "" );

    static_assert(std::to_integer<int>(test(b2)) ==  8, "" );
    static_assert(std::to_integer<int>(test(b3)) == 12, "" );

}
