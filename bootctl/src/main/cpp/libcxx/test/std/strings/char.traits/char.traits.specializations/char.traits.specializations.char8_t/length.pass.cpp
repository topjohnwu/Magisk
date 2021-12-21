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

// static constexpr size_t length(const char_type* s);

#include <string>
#include <cassert>

#include "test_macros.h"

#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
constexpr bool test_constexpr()
{
    return std::char_traits<char8_t>::length(u8"") == 0
        && std::char_traits<char8_t>::length(u8"abcd") == 4;
}

int main()
{
    assert(std::char_traits<char8_t>::length(u8"") == 0);
    assert(std::char_traits<char8_t>::length(u8"a") == 1);
    assert(std::char_traits<char8_t>::length(u8"aa") == 2);
    assert(std::char_traits<char8_t>::length(u8"aaa") == 3);
    assert(std::char_traits<char8_t>::length(u8"aaaa") == 4);

    static_assert(test_constexpr(), "");
}
#else
int main() { }
#endif
