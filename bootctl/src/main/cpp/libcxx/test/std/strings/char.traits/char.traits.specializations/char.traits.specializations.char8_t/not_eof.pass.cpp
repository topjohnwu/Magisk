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

// static constexpr int_type not_eof(int_type c);

#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    assert(std::char_traits<char8_t>::not_eof(u8'a') == u8'a');
    assert(std::char_traits<char8_t>::not_eof(u8'A') == u8'A');
    assert(std::char_traits<char8_t>::not_eof(0) == 0);
    assert(std::char_traits<char8_t>::not_eof(std::char_traits<char8_t>::eof()) !=
           std::char_traits<char8_t>::eof());
#endif
}
