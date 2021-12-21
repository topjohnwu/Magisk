//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// constexpr int compare(const charT* s) const;

#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "constexpr_char_traits.hpp"

int sign ( int x ) { return x > 0 ? 1 : ( x < 0 ? -1 : 0 ); }

template<typename CharT>
void test1 ( std::basic_string_view<CharT> sv1, const CharT *s, int expected ) {
    assert ( sign( sv1.compare(s)) == sign(expected));
}

template<typename CharT>
void
test( const CharT *s1, const CharT *s2, int expected)
{
    typedef std::basic_string_view<CharT> string_view_t;
    string_view_t sv1 ( s1 );
    test1 ( sv1, s2, expected );
}

int main()
{
    {
    test("", "", 0);
    test("", "abcde", -5);
    test("", "abcdefghij", -10);
    test("", "abcdefghijklmnopqrst", -20);
    test("abcde", "", 5);
    test("abcde", "abcde", 0);
    test("abcde", "abcdefghij", -5);
    test("abcde", "abcdefghijklmnopqrst", -15);
    test("abcdefghij", "", 10);
    test("abcdefghij", "abcde", 5);
    test("abcdefghij", "abcdefghij", 0);
    test("abcdefghij", "abcdefghijklmnopqrst", -10);
    test("abcdefghijklmnopqrst", "", 20);
    test("abcdefghijklmnopqrst", "abcde", 15);
    test("abcdefghijklmnopqrst", "abcdefghij", 10);
    test("abcdefghijklmnopqrst", "abcdefghijklmnopqrst", 0);
    }

    {
    test(L"", L"", 0);
    test(L"", L"abcde", -5);
    test(L"", L"abcdefghij", -10);
    test(L"", L"abcdefghijklmnopqrst", -20);
    test(L"abcde", L"", 5);
    test(L"abcde", L"abcde", 0);
    test(L"abcde", L"abcdefghij", -5);
    test(L"abcde", L"abcdefghijklmnopqrst", -15);
    test(L"abcdefghij", L"", 10);
    test(L"abcdefghij", L"abcde", 5);
    test(L"abcdefghij", L"abcdefghij", 0);
    test(L"abcdefghij", L"abcdefghijklmnopqrst", -10);
    test(L"abcdefghijklmnopqrst", L"", 20);
    test(L"abcdefghijklmnopqrst", L"abcde", 15);
    test(L"abcdefghijklmnopqrst", L"abcdefghij", 10);
    test(L"abcdefghijklmnopqrst", L"abcdefghijklmnopqrst", 0);
    }

#if TEST_STD_VER >= 11
    {
    test(U"", U"", 0);
    test(U"", U"abcde", -5);
    test(U"", U"abcdefghij", -10);
    test(U"", U"abcdefghijklmnopqrst", -20);
    test(U"abcde", U"", 5);
    test(U"abcde", U"abcde", 0);
    test(U"abcde", U"abcdefghij", -5);
    test(U"abcde", U"abcdefghijklmnopqrst", -15);
    test(U"abcdefghij", U"", 10);
    test(U"abcdefghij", U"abcde", 5);
    test(U"abcdefghij", U"abcdefghij", 0);
    test(U"abcdefghij", U"abcdefghijklmnopqrst", -10);
    test(U"abcdefghijklmnopqrst", U"", 20);
    test(U"abcdefghijklmnopqrst", U"abcde", 15);
    test(U"abcdefghijklmnopqrst", U"abcdefghij", 10);
    test(U"abcdefghijklmnopqrst", U"abcdefghijklmnopqrst", 0);
    }

    {
    test(u"", u"", 0);
    test(u"", u"abcde", -5);
    test(u"", u"abcdefghij", -10);
    test(u"", u"abcdefghijklmnopqrst", -20);
    test(u"abcde", u"", 5);
    test(u"abcde", u"abcde", 0);
    test(u"abcde", u"abcdefghij", -5);
    test(u"abcde", u"abcdefghijklmnopqrst", -15);
    test(u"abcdefghij", u"", 10);
    test(u"abcdefghij", u"abcde", 5);
    test(u"abcdefghij", u"abcdefghij", 0);
    test(u"abcdefghij", u"abcdefghijklmnopqrst", -10);
    test(u"abcdefghijklmnopqrst", u"", 20);
    test(u"abcdefghijklmnopqrst", u"abcde", 15);
    test(u"abcdefghijklmnopqrst", u"abcdefghij", 10);
    test(u"abcdefghijklmnopqrst", u"abcdefghijklmnopqrst", 0);
    }
#endif

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr SV  sv1;
    constexpr SV  sv2 { "abcde", 5 };
    static_assert ( sv1.compare("") == 0, "" );
    static_assert ( sv1.compare("abcde") == -1, "" );
    static_assert ( sv2.compare("") == 1, "" );
    static_assert ( sv2.compare("abcde") == 0, "" );
    }
#endif
}
