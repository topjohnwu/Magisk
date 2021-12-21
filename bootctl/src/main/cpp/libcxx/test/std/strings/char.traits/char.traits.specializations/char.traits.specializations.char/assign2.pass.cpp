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

// static constexpr void assign(char_type& c1, const char_type& c2); // constexpr in C++17
// constexpr in C++17

#include <string>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 14
constexpr bool test_constexpr()
{
    char c = '1';
    std::char_traits<char>::assign(c, 'a');
    return c == 'a';
}
#endif

int main()
{
    char c = '\0';
    std::char_traits<char>::assign(c, 'a');
    assert(c == 'a');

#if TEST_STD_VER > 14
    static_assert(test_constexpr(), "" );
#endif
}
