//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<char16_t>

// static char_type* copy(char_type* s1, const char_type* s2, size_t n);

#include <string>
#include <cassert>

int main()
{
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    char16_t s1[] = {1, 2, 3};
    char16_t s2[3] = {0};
    assert(std::char_traits<char16_t>::copy(s2, s1, 3) == s2);
    assert(s2[0] == char16_t(1));
    assert(s2[1] == char16_t(2));
    assert(s2[2] == char16_t(3));
    assert(std::char_traits<char16_t>::copy(NULL, s1, 0) == NULL);
    assert(std::char_traits<char16_t>::copy(s1, NULL, 0) == s1);
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
}
