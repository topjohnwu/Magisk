//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<wchar_t>

// static int compare(const char_type* s1, const char_type* s2, size_t n);
// constexpr in C++17

#include <string>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 14
constexpr bool test_constexpr()
{
    return std::char_traits<wchar_t>::compare(L"123", L"223", 3) < 0
        && std::char_traits<wchar_t>::compare(L"223", L"123", 3) > 0
        && std::char_traits<wchar_t>::compare(L"123", L"123", 3) == 0;
}
#endif

int main()
{
    assert(std::char_traits<wchar_t>::compare(L"", L"", 0) == 0);
    assert(std::char_traits<wchar_t>::compare(NULL, NULL, 0) == 0);

    assert(std::char_traits<wchar_t>::compare(L"1", L"1", 1) == 0);
    assert(std::char_traits<wchar_t>::compare(L"1", L"2", 1) < 0);
    assert(std::char_traits<wchar_t>::compare(L"2", L"1", 1) > 0);

    assert(std::char_traits<wchar_t>::compare(L"12", L"12", 2) == 0);
    assert(std::char_traits<wchar_t>::compare(L"12", L"13", 2) < 0);
    assert(std::char_traits<wchar_t>::compare(L"12", L"22", 2) < 0);
    assert(std::char_traits<wchar_t>::compare(L"13", L"12", 2) > 0);
    assert(std::char_traits<wchar_t>::compare(L"22", L"12", 2) > 0);

    assert(std::char_traits<wchar_t>::compare(L"123", L"123", 3) == 0);
    assert(std::char_traits<wchar_t>::compare(L"123", L"223", 3) < 0);
    assert(std::char_traits<wchar_t>::compare(L"123", L"133", 3) < 0);
    assert(std::char_traits<wchar_t>::compare(L"123", L"124", 3) < 0);
    assert(std::char_traits<wchar_t>::compare(L"223", L"123", 3) > 0);
    assert(std::char_traits<wchar_t>::compare(L"133", L"123", 3) > 0);
    assert(std::char_traits<wchar_t>::compare(L"124", L"123", 3) > 0);

#if TEST_STD_VER > 14
    static_assert(test_constexpr(), "" );
#endif
}
