//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<char32_t>

// typedef char32_t       char_type;
// typedef uint_least32_t int_type;
// typedef streamoff      off_type;
// typedef u32streampos   pos_type;
// typedef mbstate_t      state_type;

#include <string>
#include <type_traits>
#include <cstdint>

int main()
{
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    static_assert((std::is_same<std::char_traits<char32_t>::char_type, char32_t>::value), "");
    static_assert((std::is_same<std::char_traits<char32_t>::int_type, std::uint_least32_t>::value), "");
    static_assert((std::is_same<std::char_traits<char32_t>::off_type, std::streamoff>::value), "");
    static_assert((std::is_same<std::char_traits<char32_t>::pos_type, std::u32streampos>::value), "");
    static_assert((std::is_same<std::char_traits<char32_t>::state_type, std::mbstate_t>::value), "");
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
}
