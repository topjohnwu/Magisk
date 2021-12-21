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

// static const char_type* find(const char_type* s, size_t n, const char_type& a);
// constexpr in C++17

#include <string>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 14
constexpr bool test_constexpr()
{
    constexpr const char16_t *p = u"123";
    return std::char_traits<char16_t>::find(p, 3, u'1') == p
        && std::char_traits<char16_t>::find(p, 3, u'2') == p + 1
        && std::char_traits<char16_t>::find(p, 3, u'3') == p + 2
        && std::char_traits<char16_t>::find(p, 3, u'4') == nullptr;
}
#endif

int main()
{
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    char16_t s1[] = {1, 2, 3};
    assert(std::char_traits<char16_t>::find(s1, 3, char16_t(1)) == s1);
    assert(std::char_traits<char16_t>::find(s1, 3, char16_t(2)) == s1+1);
    assert(std::char_traits<char16_t>::find(s1, 3, char16_t(3)) == s1+2);
    assert(std::char_traits<char16_t>::find(s1, 3, char16_t(4)) == 0);
    assert(std::char_traits<char16_t>::find(s1, 3, char16_t(0)) == 0);
    assert(std::char_traits<char16_t>::find(NULL, 0, char16_t(0)) == 0);

#if TEST_STD_VER > 14
    static_assert(test_constexpr(), "" );
#endif
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
}
