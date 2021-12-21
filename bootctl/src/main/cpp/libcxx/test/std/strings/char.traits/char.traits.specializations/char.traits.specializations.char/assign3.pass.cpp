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

// static char_type* assign(char_type* s, size_t n, char_type a);

#include <string>
#include <cassert>

int main()
{
    char s2[3] = {0};
    assert(std::char_traits<char>::assign(s2, 3, char(5)) == s2);
    assert(s2[0] == char(5));
    assert(s2[1] == char(5));
    assert(s2[2] == char(5));
    assert(std::char_traits<char>::assign(NULL, 0, char(5)) == NULL);
}
