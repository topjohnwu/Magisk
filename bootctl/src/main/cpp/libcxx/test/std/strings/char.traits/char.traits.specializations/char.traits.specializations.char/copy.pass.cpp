//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<char>

// static char_type* copy(char_type* s1, const char_type* s2, size_t n);

#include <string>
#include <cassert>

int main()
{
    char s1[] = {1, 2, 3};
    char s2[3] = {0};
    assert(std::char_traits<char>::copy(s2, s1, 3) == s2);
    assert(s2[0] == char(1));
    assert(s2[1] == char(2));
    assert(s2[2] == char(3));
    assert(std::char_traits<char>::copy(NULL, s1, 0) == NULL);
    assert(std::char_traits<char>::copy(s1, NULL, 0) == s1);
}
