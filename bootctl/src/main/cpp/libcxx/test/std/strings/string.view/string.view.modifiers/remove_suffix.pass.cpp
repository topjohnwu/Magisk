//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


// <string_view>

// void remove_suffix(size_type _n)

#include <string_view>
#include <cassert>

#include "test_macros.h"

template<typename CharT>
void test ( const CharT *s, size_t len ) {
    typedef std::basic_string_view<CharT> SV;
    {
    SV sv1 ( s );
    assert ( sv1.size() == len );
    assert ( sv1.data() == s );

    if ( len > 0 ) {
        sv1.remove_suffix ( 1 );
        assert ( sv1.size() == (len - 1));
        assert ( sv1.data() == s);
        sv1.remove_suffix ( len - 1 );
        }

    assert ( sv1.size() == 0 );
    sv1.remove_suffix ( 0 );
    assert ( sv1.size() == 0 );
    }

}

#if TEST_STD_VER > 11
constexpr size_t test_ce ( size_t n, size_t k ) {
    typedef std::basic_string_view<char> SV;
    SV sv1{ "ABCDEFGHIJKL", n };
    sv1.remove_suffix ( k );
    return sv1.size();
}
#endif

int main () {
    test ( "ABCDE", 5 );
    test ( "a", 1 );
    test ( "", 0 );

    test ( L"ABCDE", 5 );
    test ( L"a", 1 );
    test ( L"", 0 );

#if TEST_STD_VER >= 11
    test ( u"ABCDE", 5 );
    test ( u"a", 1 );
    test ( u"", 0 );

    test ( U"ABCDE", 5 );
    test ( U"a", 1 );
    test ( U"", 0 );
#endif

#if TEST_STD_VER > 11
    {
    static_assert ( test_ce ( 5, 0 ) == 5, "" );
    static_assert ( test_ce ( 5, 1 ) == 4, "" );
    static_assert ( test_ce ( 5, 5 ) == 0, "" );
    static_assert ( test_ce ( 9, 3 ) == 6, "" );
    }
#endif
}
