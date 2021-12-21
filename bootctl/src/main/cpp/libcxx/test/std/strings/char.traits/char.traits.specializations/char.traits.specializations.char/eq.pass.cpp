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

// static constexpr bool eq(char_type c1, char_type c2);

#include <string>
#include <cassert>

int main()
{
    assert(std::char_traits<char>::eq('a', 'a'));
    assert(!std::char_traits<char>::eq('a', 'A'));
}
