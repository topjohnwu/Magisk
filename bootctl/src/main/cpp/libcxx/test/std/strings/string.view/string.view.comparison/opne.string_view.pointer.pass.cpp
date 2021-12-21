//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<class charT, class traits>
//   constexpr bool operator!=(basic_string_view<charT,traits> lhs, const charT* rhs);
// template<class charT, class traits>
//   constexpr bool operator!=(const charT* lhs, basic_string_view<charT,traits> rhs);

#include <string_view>
#include <cassert>

#include "test_macros.h"
#include "constexpr_char_traits.hpp"

template <class S>
void
test(S lhs, const typename S::value_type* rhs, bool x)
{
    assert((lhs != rhs) == x);
    assert((rhs != lhs) == x);
}

int main()
{
    {
    typedef std::string_view S;
    test(S(""), "", false);
    test(S(""), "abcde", true);
    test(S(""), "abcdefghij", true);
    test(S(""), "abcdefghijklmnopqrst", true);
    test(S("abcde"), "", true);
    test(S("abcde"), "abcde", false);
    test(S("abcde"), "abcdefghij", true);
    test(S("abcde"), "abcdefghijklmnopqrst", true);
    test(S("abcdefghij"), "", true);
    test(S("abcdefghij"), "abcde", true);
    test(S("abcdefghij"), "abcdefghij", false);
    test(S("abcdefghij"), "abcdefghijklmnopqrst", true);
    test(S("abcdefghijklmnopqrst"), "", true);
    test(S("abcdefghijklmnopqrst"), "abcde", true);
    test(S("abcdefghijklmnopqrst"), "abcdefghij", true);
    test(S("abcdefghijklmnopqrst"), "abcdefghijklmnopqrst", false);
    }

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr SV  sv1;
    constexpr SV  sv2 { "abcde", 5 };

    static_assert (!(sv1     != ""), "" );
    static_assert (!(""      != sv1), "" );
    static_assert (  sv1     != "abcde", "" );
    static_assert (  "abcde" != sv1, "" );

    static_assert (!(sv2      != "abcde"), "" );
    static_assert (!("abcde"  != sv2), "" );
    static_assert (  sv2      != "abcde0",   "" );
    static_assert (  "abcde0" != sv2, "" );
    }
#endif
}
