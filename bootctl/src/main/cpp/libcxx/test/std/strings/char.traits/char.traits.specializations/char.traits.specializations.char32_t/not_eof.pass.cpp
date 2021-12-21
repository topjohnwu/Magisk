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

// static constexpr int_type not_eof(int_type c);

#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
#if TEST_STD_VER >= 11
    assert(std::char_traits<char32_t>::not_eof(U'a') == U'a');
    assert(std::char_traits<char32_t>::not_eof(U'A') == U'A');
#endif
    assert(std::char_traits<char32_t>::not_eof(0) == 0);
    assert(std::char_traits<char32_t>::not_eof(std::char_traits<char32_t>::eof()) !=
           std::char_traits<char32_t>::eof());
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
}
