//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// enum class align_val_t : size_t {}

// UNSUPPORTED: c++98, c++03, c++11, c++14

#include <new>

#include "test_macros.h"

int main() {
  {
    static_assert(std::is_enum<std::align_val_t>::value, "");
    static_assert(std::is_same<std::underlying_type<std::align_val_t>::type, std::size_t>::value, "");
    static_assert(!std::is_constructible<std::align_val_t, std::size_t>::value, "");
    static_assert(!std::is_constructible<std::size_t, std::align_val_t>::value, "");
  }
  {
    constexpr auto a = std::align_val_t(0);
    constexpr auto b = std::align_val_t(32);
    constexpr auto c = std::align_val_t(-1);
    static_assert(a != b, "");
    static_assert(a == std::align_val_t(0), "");
    static_assert(b == std::align_val_t(32), "");
    static_assert(static_cast<std::size_t>(c) == (std::size_t)-1, "");
  }
}
