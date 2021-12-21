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

// static constexpr bool lt(char_type c1, char_type c2);

#include <string>
#include <cassert>

int main()
{
    assert(!std::char_traits<wchar_t>::lt(L'a', L'a'));
    assert( std::char_traits<wchar_t>::lt(L'A', L'a'));
}
