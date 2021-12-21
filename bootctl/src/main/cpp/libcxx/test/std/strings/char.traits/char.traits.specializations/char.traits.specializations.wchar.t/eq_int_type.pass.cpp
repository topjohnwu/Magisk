//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<wchar_t>

// static constexpr bool eq_int_type(int_type c1, int_type c2);

#include <string>
#include <cassert>

int main()
{
    assert( std::char_traits<wchar_t>::eq_int_type(L'a', L'a'));
    assert(!std::char_traits<wchar_t>::eq_int_type(L'a', L'A'));
    assert(!std::char_traits<wchar_t>::eq_int_type(std::char_traits<wchar_t>::eof(), L'A'));
    assert( std::char_traits<wchar_t>::eq_int_type(std::char_traits<wchar_t>::eof(),
                                                   std::char_traits<wchar_t>::eof()));
}
