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

// static constexpr char_type to_char_type(int_type c);

#include <string>
#include <cassert>

int main()
{
    assert(std::char_traits<char>::to_char_type('a') == 'a');
    assert(std::char_traits<char>::to_char_type('A') == 'A');
    assert(std::char_traits<char>::to_char_type(0) == 0);
}
