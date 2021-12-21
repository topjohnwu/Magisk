//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// size_type copy(charT* s, size_type n, size_type pos = 0) const;

// Throws: out_of_range if pos > size().
// Remarks: Let rlen be the smaller of n and size() - pos.
// Requires: [s, s+rlen) is a valid range.
// Effects: Equivalent to std::copy_n(begin() + pos, rlen, s).
// Returns: rlen.


#include <string_view>
#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "test_macros.h"

template<typename CharT>
void test1 ( std::basic_string_view<CharT> sv, size_t n, size_t pos ) {
    const size_t rlen = std::min ( n, sv.size() - pos );

    CharT *dest1 = new CharT [rlen + 1];    dest1[rlen] = 0;
    CharT *dest2 = new CharT [rlen + 1];    dest2[rlen] = 0;

    if (pos > sv.size()) {
#ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            sv.copy(dest1, n, pos);
            assert(false);
        } catch (const std::out_of_range&) {
        } catch (...) {
            assert(false);
        }
#endif
    } else {
        sv.copy(dest1, n, pos);
        std::copy_n(sv.begin() + pos, rlen, dest2);
        for ( size_t i = 0; i <= rlen; ++i )
            assert ( dest1[i] == dest2[i] );
    }
    delete [] dest1;
    delete [] dest2;
}


template<typename CharT>
void test ( const CharT *s ) {
    typedef std::basic_string_view<CharT> string_view_t;

    string_view_t sv1 ( s );

    test1(sv1,  0, 0);
    test1(sv1,  1, 0);
    test1(sv1, 20, 0);
    test1(sv1, sv1.size(), 0);
    test1(sv1, 20, string_view_t::npos);

    test1(sv1,   0, 3);
    test1(sv1,   2, 3);
    test1(sv1, 100, 3);
    test1(sv1, 100, string_view_t::npos);

    test1(sv1, sv1.size(), string_view_t::npos);

    test1(sv1, sv1.size() + 1, 0);
    test1(sv1, sv1.size() + 1, 1);
    test1(sv1, sv1.size() + 1, string_view_t::npos);

}

int main () {
    test ( "ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE" );
    test ( "ABCDE");
    test ( "a" );
    test ( "" );

    test ( L"ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE" );
    test ( L"ABCDE" );
    test ( L"a" );
    test ( L"" );

#if TEST_STD_VER >= 11
    test ( u"ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE" );
    test ( u"ABCDE" );
    test ( u"a" );
    test ( u"" );

    test ( U"ABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDEABCDE" );
    test ( U"ABCDE" );
    test ( U"a" );
    test ( U"" );
#endif
}
