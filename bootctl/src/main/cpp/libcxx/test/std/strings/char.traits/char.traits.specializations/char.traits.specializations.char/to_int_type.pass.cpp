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

// static constexpr int_type to_int_type(char_type c);

#include <string>
#include <cassert>

int main()
{
    assert(std::char_traits<char>::to_int_type('a') == 'a');
    assert(std::char_traits<char>::to_int_type('A') == 'A');
    assert(std::char_traits<char>::to_int_type(0) == 0);
}
