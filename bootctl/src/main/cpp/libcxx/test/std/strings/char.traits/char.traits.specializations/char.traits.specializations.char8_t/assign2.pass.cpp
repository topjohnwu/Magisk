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

// static constexpr void assign(char_type& c1, const char_type& c2);

#include <string>
#include <cassert>

#include "test_macros.h"

#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
constexpr bool test_constexpr()
{
    char8_t c = u'1';
    std::char_traits<char8_t>::assign(c, u'a');
    return c == u'a';
}

int main()
{
    char8_t c = u8'\0';
    std::char_traits<char8_t>::assign(c, u8'a');
    assert(c == u8'a');

    static_assert(test_constexpr(), "");
}
#else
int main () {}
#endif
