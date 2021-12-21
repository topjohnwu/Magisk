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

// static char_type* assign(char_type* s, size_t n, char_type a);

#include <string>
#include <cassert>

int main()
{
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    char8_t s2[3] = {0};
    assert(std::char_traits<char8_t>::assign(s2, 3, char8_t(5)) == s2);
    assert(s2[0] == char8_t(5));
    assert(s2[1] == char8_t(5));
    assert(s2[2] == char8_t(5));
    assert(std::char_traits<char8_t>::assign(NULL, 0, char8_t(5)) == NULL);
#endif
}
