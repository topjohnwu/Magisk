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

// static constexpr bool lt(char_type c1, char_type c2);

#include <string>
#include <cassert>

int main()
{
    assert( std::char_traits<char>::lt('\0', 'A'));
    assert(!std::char_traits<char>::lt('A', '\0'));

    assert(!std::char_traits<char>::lt('a', 'a'));
    assert( std::char_traits<char>::lt('A', 'a'));
    assert(!std::char_traits<char>::lt('a', 'A'));

    assert( std::char_traits<char>::lt('a', 'z'));
    assert( std::char_traits<char>::lt('A', 'Z'));

    assert( std::char_traits<char>::lt(' ', 'A'));
    assert( std::char_traits<char>::lt('A', '~'));
}
