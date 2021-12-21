//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template<class Codecvt, class Elem = wchar_t,
//          class Wide_alloc = allocator<Elem>,
//          class Byte_alloc = allocator<char>>
// class wstring_convert
// {
// public:
//     typedef basic_string<char, char_traits<char>, Byte_alloc> byte_string;
//     typedef basic_string<Elem, char_traits<Elem>, Wide_alloc> wide_string;
//     typedef typename Codecvt::state_type                      state_type;
//     typedef typename wide_string::traits_type::int_type       int_type;

#include <locale>
#include <codecvt>

int main()
{
    {
        typedef std::wstring_convert<std::codecvt_utf8<wchar_t> > myconv;
        static_assert((std::is_same<myconv::byte_string, std::string>::value), "");
        static_assert((std::is_same<myconv::wide_string, std::wstring>::value), "");
        static_assert((std::is_same<myconv::state_type, std::mbstate_t>::value), "");
        static_assert((std::is_same<myconv::int_type, std::char_traits<wchar_t>::int_type>::value), "");
    }
}
