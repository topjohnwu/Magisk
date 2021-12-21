//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class _CharT>
// class messages
//     : public locale::facet,
//       public messages_base
// {
// public:
//     typedef _CharT               char_type;
//     typedef basic_string<_CharT> string_type;

#include <locale>
#include <type_traits>

int main()
{
    static_assert((std::is_base_of<std::locale::facet, std::messages<char> >::value), "");
    static_assert((std::is_base_of<std::messages_base, std::messages<char> >::value), "");
    static_assert((std::is_base_of<std::locale::facet, std::messages<wchar_t> >::value), "");
    static_assert((std::is_base_of<std::messages_base, std::messages<wchar_t> >::value), "");
    static_assert((std::is_same<std::messages<char>::char_type, char>::value), "");
    static_assert((std::is_same<std::messages<wchar_t>::char_type, wchar_t>::value), "");
    static_assert((std::is_same<std::messages<char>::string_type, std::string>::value), "");
    static_assert((std::is_same<std::messages<wchar_t>::string_type, std::wstring>::value), "");
}
