//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class CharT, class InputIterator = istreambuf_iterator<CharT> >
// class money_get
//     : public locale::facet
// {
// public:
//     typedef CharT                   char_type;
//     typedef InputIterator           iter_type;
//     typedef basic_string<char_type> string_type;

#include <locale>
#include <type_traits>

int main()
{
    static_assert((std::is_base_of<std::locale::facet, std::money_get<char> >::value), "");
    static_assert((std::is_base_of<std::locale::facet, std::money_get<wchar_t> >::value), "");
    static_assert((std::is_same<std::money_get<char>::char_type, char>::value), "");
    static_assert((std::is_same<std::money_get<wchar_t>::char_type, wchar_t>::value), "");
    static_assert((std::is_same<std::money_get<char>::iter_type, std::istreambuf_iterator<char> >::value), "");
    static_assert((std::is_same<std::money_get<wchar_t>::iter_type, std::istreambuf_iterator<wchar_t> >::value), "");
    static_assert((std::is_same<std::money_get<char>::string_type, std::string>::value), "");
    static_assert((std::is_same<std::money_get<wchar_t>::string_type, std::wstring>::value), "");
}
