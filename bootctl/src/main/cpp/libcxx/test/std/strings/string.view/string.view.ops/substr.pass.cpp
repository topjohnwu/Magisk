//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// constexpr basic_string_view substr(size_type pos = 0, size_type n = npos) const;

// Throws: out_of_range if pos > size().
// Effects: Determines the effective length rlen of the string to reference as the smaller of n and size() - pos.
// Returns: basic_string_view(data()+pos, rlen).

#include <string_view>
#include <algorithm>
#include <stdexcept>
#include <cassert>

#include "test_macros.h"

template<typename CharT>
void test1(std::basic_string_view<CharT> sv, size_t n, size_t pos) {
    std::basic_string_view<CharT> sv1;
#ifdef TEST_HAS_NO_EXCEPTIONS
    if (pos > sv.size())
        return ;  // would throw if exceptions were enabled
    sv1 = sv.substr(pos, n);
#else
    try {
        sv1 = sv.substr(pos, n);
        assert(pos <= sv.size());
    }
    catch (const std::out_of_range&) {
        assert(pos > sv.size());
        return ;
    }
#endif
    const size_t rlen = std::min(n, sv.size() - pos);
    assert (sv1.size() == rlen);
    for (size_t i = 0; i < rlen; ++i)
        assert(sv[pos+i] == sv1[i]);
}


template<typename CharT>
void test ( const CharT *s ) {
    typedef std::basic_string_view<CharT> string_view_t;

    string_view_t sv1 ( s );

    test1(sv1,  0, 0);
    test1(sv1,  1, 0);
    test1(sv1, 20, 0);
    test1(sv1, sv1.size(), 0);

    test1(sv1,   0, 3);
    test1(sv1,   2, 3);
    test1(sv1, 100, 3);

    test1(sv1, 0, string_view_t::npos);
    test1(sv1, 2, string_view_t::npos);
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

#if TEST_STD_VER > 11
    {
    constexpr std::string_view sv1 { "ABCDE", 5 };

    {
    constexpr std::string_view sv2 = sv1.substr ( 0, 3 );
    static_assert ( sv2.size() == 3, "" );
    static_assert ( sv2[0] == 'A', "" );
    static_assert ( sv2[1] == 'B', "" );
    static_assert ( sv2[2] == 'C', "" );
    }

    {
    constexpr std::string_view sv2 = sv1.substr ( 3, 0 );
    static_assert ( sv2.size() == 0, "" );
    }

    {
    constexpr std::string_view sv2 = sv1.substr ( 3, 3 );
    static_assert ( sv2.size() == 2, "" );
    static_assert ( sv2[0] == 'D', "" );
    static_assert ( sv2[1] == 'E', "" );
    }
    }
#endif
}
