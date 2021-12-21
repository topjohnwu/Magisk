//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class strstream
//     : public basic_iostream<char>
// {
// public:
//     // Types
//     typedef char                        char_type;
//     typedef char_traits<char>::int_type int_type;
//     typedef char_traits<char>::pos_type pos_type;
//     typedef char_traits<char>::off_type off_type;

#include <strstream>
#include <type_traits>

int main()
{
    static_assert((std::is_base_of<std::iostream, std::strstream>::value), "");
    static_assert((std::is_same<std::strstream::char_type, char>::value), "");
    static_assert((std::is_same<std::strstream::int_type, std::char_traits<char>::int_type>::value), "");
    static_assert((std::is_same<std::strstream::pos_type, std::char_traits<char>::pos_type>::value), "");
    static_assert((std::is_same<std::strstream::off_type, std::char_traits<char>::off_type>::value), "");
}
