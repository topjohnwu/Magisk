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

// typedef wchar_t   char_type;
// typedef int       int_type;
// typedef streamoff off_type;
// typedef streampos pos_type;
// typedef mbstate_t state_type;

#include <string>
#include <type_traits>

int main()
{
    static_assert((std::is_same<std::char_traits<wchar_t>::char_type, wchar_t>::value), "");
    static_assert((std::is_same<std::char_traits<wchar_t>::int_type, std::wint_t>::value), "");
    static_assert((std::is_same<std::char_traits<wchar_t>::off_type, std::streamoff>::value), "");
    static_assert((std::is_same<std::char_traits<wchar_t>::pos_type, std::wstreampos>::value), "");
    static_assert((std::is_same<std::char_traits<wchar_t>::state_type, std::mbstate_t>::value), "");
}
