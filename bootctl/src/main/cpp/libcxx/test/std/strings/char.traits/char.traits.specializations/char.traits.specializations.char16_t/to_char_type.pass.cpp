//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<char16_t>

// static constexpr char_type to_char_type(int_type c);

#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
#if TEST_STD_VER >= 11
    assert(std::char_traits<char16_t>::to_char_type(u'a') == u'a');
    assert(std::char_traits<char16_t>::to_char_type(u'A') == u'A');
#endif
    assert(std::char_traits<char16_t>::to_char_type(0) == 0);
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
}
