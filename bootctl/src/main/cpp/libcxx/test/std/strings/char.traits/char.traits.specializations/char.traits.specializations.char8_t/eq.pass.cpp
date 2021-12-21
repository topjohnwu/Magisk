//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <string>

// template<> struct char_traits<char8_t>

// static constexpr bool eq(char_type c1, char_type c2);

#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    assert( std::char_traits<char8_t>::eq(u8'a', u8'a'));
    assert(!std::char_traits<char8_t>::eq(u8'a', u8'A'));
#endif
}
