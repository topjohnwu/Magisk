// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT>
// struct regex_traits
// {
// public:
//     typedef charT                   char_type;
//     typedef basic_string<char_type> string_type;
//     typedef locale                  locale_type;

#include <regex>
#include <type_traits>
#include "test_macros.h"

int main()
{
    static_assert((std::is_same<std::regex_traits<char>::char_type, char>::value), "");
    static_assert((std::is_same<std::regex_traits<char>::string_type, std::string>::value), "");
    static_assert((std::is_same<std::regex_traits<char>::locale_type, std::locale>::value), "");
    static_assert((std::is_same<std::regex_traits<wchar_t>::char_type, wchar_t>::value), "");
    static_assert((std::is_same<std::regex_traits<wchar_t>::string_type, std::wstring>::value), "");
    static_assert((std::is_same<std::regex_traits<wchar_t>::locale_type, std::locale>::value), "");
}
