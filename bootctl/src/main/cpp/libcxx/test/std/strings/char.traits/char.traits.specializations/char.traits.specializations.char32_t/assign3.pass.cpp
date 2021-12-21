//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<char32_t>

// static char_type* assign(char_type* s, size_t n, char_type a);

#include <string>
#include <cassert>

int main()
{
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    char32_t s2[3] = {0};
    assert(std::char_traits<char32_t>::assign(s2, 3, char32_t(5)) == s2);
    assert(s2[0] == char32_t(5));
    assert(s2[1] == char32_t(5));
    assert(s2[2] == char32_t(5));
    assert(std::char_traits<char32_t>::assign(NULL, 0, char32_t(5)) == NULL);
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
}
