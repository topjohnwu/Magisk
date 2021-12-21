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

// static size_t length(const char_type* s);
// constexpr in C++17

#include <string>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 14
constexpr bool test_constexpr()
{
    return std::char_traits<char>::length("") == 0
        && std::char_traits<char>::length("abcd") == 4;
}
#endif


int main()
{
    assert(std::char_traits<char>::length("") == 0);
    assert(std::char_traits<char>::length("a") == 1);
    assert(std::char_traits<char>::length("aa") == 2);
    assert(std::char_traits<char>::length("aaa") == 3);
    assert(std::char_traits<char>::length("aaaa") == 4);

#if TEST_STD_VER > 14
    static_assert(test_constexpr(), "" );
#endif
}
