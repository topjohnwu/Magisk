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
//    constexpr IntegerType to_integer(byte b) noexcept;
// This function shall not participate in overload resolution unless
//   is_integral_v<IntegerType> is true.

int main () {
    constexpr std::byte b1{static_cast<std::byte>(1)};
    constexpr std::byte b3{static_cast<std::byte>(3)};

    static_assert(noexcept(std::to_integer<int>(b1)), "" );
    static_assert(std::is_same<int, decltype(std::to_integer<int>(b1))>::value, "" );
    static_assert(std::is_same<long, decltype(std::to_integer<long>(b1))>::value, "" );
    static_assert(std::is_same<unsigned short, decltype(std::to_integer<unsigned short>(b1))>::value, "" );

    static_assert(std::to_integer<int>(b1) == 1, "");
    static_assert(std::to_integer<int>(b3) == 3, "");
}
