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

// static constexpr bool eq_int_type(int_type c1, int_type c2);

#include <string>
#include <cassert>

int main()
{
    assert( std::char_traits<char>::eq_int_type('a', 'a'));
    assert(!std::char_traits<char>::eq_int_type('a', 'A'));
    assert(!std::char_traits<char>::eq_int_type(std::char_traits<char>::eof(), 'A'));
    assert( std::char_traits<char>::eq_int_type(std::char_traits<char>::eof(),
                                                std::char_traits<char>::eof()));
}
