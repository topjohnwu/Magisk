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

// static char_type* move(char_type* s1, const char_type* s2, size_t n);

#include <string>
#include <cassert>

int main()
{
    char s1[] = {1, 2, 3};
    assert(std::char_traits<char>::move(s1, s1+1, 2) == s1);
    assert(s1[0] == char(2));
    assert(s1[1] == char(3));
    assert(s1[2] == char(3));
    s1[2] = char(0);
    assert(std::char_traits<char>::move(s1+1, s1, 2) == s1+1);
    assert(s1[0] == char(2));
    assert(s1[1] == char(2));
    assert(s1[2] == char(3));
    assert(std::char_traits<char>::move(NULL, s1, 0) == NULL);
    assert(std::char_traits<char>::move(s1, NULL, 0) == s1);
}
