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

// static constexpr int_type not_eof(int_type c);

#include <string>
#include <cassert>

int main()
{
    assert(std::char_traits<wchar_t>::not_eof(L'a') == L'a');
    assert(std::char_traits<wchar_t>::not_eof(L'A') == L'A');
    assert(std::char_traits<wchar_t>::not_eof(0) == 0);
    assert(std::char_traits<wchar_t>::not_eof(std::char_traits<wchar_t>::eof()) !=
           std::char_traits<wchar_t>::eof());
}
