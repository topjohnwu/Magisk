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

// static int compare(const char_type* s1, const char_type* s2, size_t n);
// constexpr in C++17

#include <string>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 14
constexpr bool test_constexpr()
{
    return std::char_traits<char>::compare("123", "223", 3) < 0
        && std::char_traits<char>::compare("223", "123", 3) > 0
        && std::char_traits<char>::compare("123", "123", 3) == 0;
}
#endif

int main()
{
    assert(std::char_traits<char>::compare("", "", 0) == 0);
    assert(std::char_traits<char>::compare(NULL, NULL, 0) == 0);

    assert(std::char_traits<char>::compare("1", "1", 1) == 0);
    assert(std::char_traits<char>::compare("1", "2", 1) < 0);
    assert(std::char_traits<char>::compare("2", "1", 1) > 0);

    assert(std::char_traits<char>::compare("12", "12", 2) == 0);
    assert(std::char_traits<char>::compare("12", "13", 2) < 0);
    assert(std::char_traits<char>::compare("12", "22", 2) < 0);
    assert(std::char_traits<char>::compare("13", "12", 2) > 0);
    assert(std::char_traits<char>::compare("22", "12", 2) > 0);

    assert(std::char_traits<char>::compare("123", "123", 3) == 0);
    assert(std::char_traits<char>::compare("123", "223", 3) < 0);
    assert(std::char_traits<char>::compare("123", "133", 3) < 0);
    assert(std::char_traits<char>::compare("123", "124", 3) < 0);
    assert(std::char_traits<char>::compare("223", "123", 3) > 0);
    assert(std::char_traits<char>::compare("133", "123", 3) > 0);
    assert(std::char_traits<char>::compare("124", "123", 3) > 0);

#if TEST_STD_VER > 14
    static_assert(test_constexpr(), "" );
#endif
}
