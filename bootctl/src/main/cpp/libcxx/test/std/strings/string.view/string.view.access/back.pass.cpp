//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


// <string_view>

// constexpr const _CharT& front();

#include <string_view>
#include <cassert>

#include "test_macros.h"

template <typename CharT>
bool test ( const CharT *s, size_t len ) {
    std::basic_string_view<CharT> sv ( s, len );
    assert ( sv.length() == len );
    assert ( sv.back() == s[len-1] );
    return &sv.back() == s + len - 1;
    }

int main () {
    assert ( test ( "ABCDE", 5 ));
    assert ( test ( "a", 1 ));

    assert ( test ( L"ABCDE", 5 ));
    assert ( test ( L"a", 1 ));

#if TEST_STD_VER >= 11
    assert ( test ( u"ABCDE", 5 ));
    assert ( test ( u"a", 1 ));

    assert ( test ( U"ABCDE", 5 ));
    assert ( test ( U"a", 1 ));
#endif

#if TEST_STD_VER >= 11
    {
    constexpr std::basic_string_view<char> sv ( "ABC", 2 );
    static_assert ( sv.length() ==  2,  "" );
    static_assert ( sv.back()  == 'B', "" );
    }
#endif
}
