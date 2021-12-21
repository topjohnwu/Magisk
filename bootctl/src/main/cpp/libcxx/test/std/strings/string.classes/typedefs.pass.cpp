//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// Test for the existence of:

// basic_string typedef names
// typedef basic_string<char>     string;
// typedef basic_string<char16_t> u16string;
// typedef basic_string<char8_t>  u8string;  // C++20
// typedef basic_string<char32_t> u32string;
// typedef basic_string<wchar_t>  wstring;

#include <string>
#include <type_traits>

#include "test_macros.h"

int main()
{
    static_assert((std::is_same<std::string, std::basic_string<char> >::value), "");
    static_assert((std::is_same<std::wstring, std::basic_string<wchar_t> >::value), "");
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    static_assert((std::is_same<std::u8string, std::basic_string<char8_t> >::value), "");
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    static_assert((std::is_same<std::u16string, std::basic_string<char16_t> >::value), "");
    static_assert((std::is_same<std::u32string, std::basic_string<char32_t> >::value), "");
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
}
