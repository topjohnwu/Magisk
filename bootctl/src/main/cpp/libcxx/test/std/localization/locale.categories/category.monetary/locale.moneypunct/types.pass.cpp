//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This test uses new symbols that were not defined in the libc++ shipped on
// darwin11 and darwin12:
// XFAIL: with_system_cxx_lib=macosx10.7
// XFAIL: with_system_cxx_lib=macosx10.8

// <locale>

// template <class _CharT, bool _International = false>
// class moneypunct
//     : public locale::facet,
//       public money_base
// {
// public:
//     typedef _CharT                  char_type;
//     typedef basic_string<char_type> string_type;
//     static const bool intl = International;

#include <locale>
#include <type_traits>

template <class T>
void test(const T &) {}

int main()
{
    static_assert((std::is_base_of<std::locale::facet, std::moneypunct<char> >::value), "");
    static_assert((std::is_base_of<std::locale::facet, std::moneypunct<wchar_t> >::value), "");
    static_assert((std::is_base_of<std::money_base, std::moneypunct<char> >::value), "");
    static_assert((std::is_base_of<std::money_base, std::moneypunct<wchar_t> >::value), "");
    static_assert((std::is_same<std::moneypunct<char>::char_type, char>::value), "");
    static_assert((std::is_same<std::moneypunct<wchar_t>::char_type, wchar_t>::value), "");
    static_assert((std::is_same<std::moneypunct<char>::string_type, std::string>::value), "");
    static_assert((std::is_same<std::moneypunct<wchar_t>::string_type, std::wstring>::value), "");

    test(std::moneypunct<char, false>::intl);
    test(std::moneypunct<char, true>::intl);
    test(std::moneypunct<wchar_t, false>::intl);
    test(std::moneypunct<wchar_t, true>::intl);
}
