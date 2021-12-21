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

// static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a);

#include <string>
#include <cassert>

#include "test_macros.h"

#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
constexpr bool test_constexpr()
{
    constexpr const char8_t *p = u8"123";
    return std::char_traits<char8_t>::find(p, 3, u8'1') == p
        && std::char_traits<char8_t>::find(p, 3, u8'2') == p + 1
        && std::char_traits<char8_t>::find(p, 3, u8'3') == p + 2
        && std::char_traits<char8_t>::find(p, 3, u8'4') == nullptr;
}

int main()
{
    char8_t s1[] = {1, 2, 3};
    assert(std::char_traits<char8_t>::find(s1, 3, char8_t(1)) == s1);
    assert(std::char_traits<char8_t>::find(s1, 3, char8_t(2)) == s1+1);
    assert(std::char_traits<char8_t>::find(s1, 3, char8_t(3)) == s1+2);
    assert(std::char_traits<char8_t>::find(s1, 3, char8_t(4)) == 0);
    assert(std::char_traits<char8_t>::find(s1, 3, char8_t(0)) == 0);
    assert(std::char_traits<char8_t>::find(NULL, 0, char8_t(0)) == 0);

    static_assert(test_constexpr(), "" );
}
#else
int main () {}
#endif
