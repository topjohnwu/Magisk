//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


// <string_view>

// constexpr const _CharT& operator[](size_type _pos) const;

#include <string_view>
#include <cassert>

#include "test_macros.h"

template <typename CharT>
void test ( const CharT *s, size_t len ) {
    std::basic_string_view<CharT> sv ( s, len );
    assert ( sv.length() == len );
    for ( size_t i = 0; i < len; ++i ) {
        assert ( sv[i] == s[i] );
        assert ( &sv[i] == s + i );
        }
    }

int main () {
    test ( "ABCDE", 5 );
    test ( "a", 1 );

    test ( L"ABCDE", 5 );
    test ( L"a", 1 );

#if TEST_STD_VER >= 11
    test ( u"ABCDE", 5 );
    test ( u"a", 1 );

    test ( U"ABCDE", 5 );
    test ( U"a", 1 );
#endif

#if TEST_STD_VER > 11
    {
    constexpr std::basic_string_view<char> sv ( "ABC", 2 );
    static_assert ( sv.length() ==  2,  "" );
    static_assert ( sv[0]  == 'A', "" );
    static_assert ( sv[1]  == 'B', "" );
    }
#endif
}
