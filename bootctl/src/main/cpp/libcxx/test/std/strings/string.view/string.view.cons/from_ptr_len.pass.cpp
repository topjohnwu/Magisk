//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


// <string_view>

//  constexpr basic_string_view(const _CharT* _s, size_type _len)
//      : __data (_s), __size(_len) {}


#include <string_view>
#include <string>
#include <cassert>

#include "test_macros.h"

template<typename CharT>
void test ( const CharT *s, size_t sz ) {
    {
    typedef std::basic_string_view<CharT> SV;
    LIBCPP_ASSERT_NOEXCEPT(SV(s, sz));

    SV sv1 ( s, sz );
    assert ( sv1.size() == sz );
    assert ( sv1.data() == s );
    }
}

int main () {

    test ( "QBCDE", 5 );
    test ( "QBCDE", 2 );
    test ( "", 0 );
#if TEST_STD_VER > 11
    {
    constexpr const char *s = "QBCDE";
    constexpr std::basic_string_view<char> sv1 ( s, 2 );
    static_assert ( sv1.size() == 2, "" );
    static_assert ( sv1.data() == s, "" );
    }
#endif

    test ( L"QBCDE", 5 );
    test ( L"QBCDE", 2 );
    test ( L"", 0 );
#if TEST_STD_VER > 11
    {
    constexpr const wchar_t *s = L"QBCDE";
    constexpr std::basic_string_view<wchar_t> sv1 ( s, 2 );
    static_assert ( sv1.size() == 2, "" );
    static_assert ( sv1.data() == s, "" );
    }
#endif

#if TEST_STD_VER >= 11
    test ( u"QBCDE", 5 );
    test ( u"QBCDE", 2 );
    test ( u"", 0 );
#if TEST_STD_VER > 11
    {
    constexpr const char16_t *s = u"QBCDE";
    constexpr std::basic_string_view<char16_t> sv1 ( s, 2 );
    static_assert ( sv1.size() == 2, "" );
    static_assert ( sv1.data() == s, "" );
    }
#endif

    test ( U"QBCDE", 5 );
    test ( U"QBCDE", 2 );
    test ( U"", 0 );
#if TEST_STD_VER > 11
    {
    constexpr const char32_t *s = U"QBCDE";
    constexpr std::basic_string_view<char32_t> sv1 ( s, 2 );
    static_assert ( sv1.size() == 2, "" );
    static_assert ( sv1.data() == s, "" );
    }
#endif
#endif
}
