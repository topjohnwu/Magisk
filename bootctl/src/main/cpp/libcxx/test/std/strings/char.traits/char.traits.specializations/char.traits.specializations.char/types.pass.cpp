//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<char>

// typedef char      char_type;
// typedef int       int_type;
// typedef streamoff off_type;
// typedef streampos pos_type;
// typedef mbstate_t state_type;

#include <string>
#include <type_traits>

int main()
{
    static_assert((std::is_same<std::char_traits<char>::char_type, char>::value), "");
    static_assert((std::is_same<std::char_traits<char>::int_type, int>::value), "");
    static_assert((std::is_same<std::char_traits<char>::off_type, std::streamoff>::value), "");
    static_assert((std::is_same<std::char_traits<char>::pos_type, std::streampos>::value), "");
    static_assert((std::is_same<std::char_traits<char>::state_type, std::mbstate_t>::value), "");
}
