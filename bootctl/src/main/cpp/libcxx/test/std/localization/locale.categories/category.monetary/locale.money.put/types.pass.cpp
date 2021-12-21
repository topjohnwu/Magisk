//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class CharT, class OutputIterator = ostreambuf_iterator<CharT> >
// class money_put
//     : public locale::facet
// {
// public:
//     typedef CharT                   char_type;
//     typedef OutputIterator          iter_type;
//     typedef basic_string<char_type> string_type;

#include <locale>
#include <type_traits>

int main()
{
    static_assert((std::is_base_of<std::locale::facet, std::money_put<char> >::value), "");
    static_assert((std::is_base_of<std::locale::facet, std::money_put<wchar_t> >::value), "");
    static_assert((std::is_same<std::money_put<char>::char_type, char>::value), "");
    static_assert((std::is_same<std::money_put<wchar_t>::char_type, wchar_t>::value), "");
    static_assert((std::is_same<std::money_put<char>::iter_type, std::ostreambuf_iterator<char> >::value), "");
    static_assert((std::is_same<std::money_put<wchar_t>::iter_type, std::ostreambuf_iterator<wchar_t> >::value), "");
    static_assert((std::is_same<std::money_put<char>::string_type, std::string>::value), "");
    static_assert((std::is_same<std::money_put<wchar_t>::string_type, std::wstring>::value), "");
}
