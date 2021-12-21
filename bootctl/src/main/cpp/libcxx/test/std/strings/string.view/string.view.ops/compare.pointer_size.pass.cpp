//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// constexpr int compare(size_type pos1, size_type n1, const charT* s) const;

#include <string_view>
#include <cassert>
#include <stdexcept>

#include "test_macros.h"
#include "constexpr_char_traits.hpp"

int sign ( int x ) { return x > 0 ? 1 : ( x < 0 ? -1 : 0 ); }

template<typename CharT>
void test1 ( std::basic_string_view<CharT> sv1,
             size_t pos1, size_t n1, const CharT *s, int expected ) {
#ifdef TEST_HAS_NO_EXCEPTIONS
    if (pos1 <= sv1.size())
        assert(sign(sv1.compare(pos1, n1, s)) == sign(expected));
#else
    try {
        assert(sign(sv1.compare(pos1, n1, s)) == sign(expected));
        assert(pos1 <= sv1.size());
    }
    catch (const std::out_of_range&) {
        assert(pos1 > sv1.size());
    }
#endif
}

template<typename CharT>
void
test( const CharT *s1, size_t pos1, size_t n1, const CharT *s2, int expected)
{
    typedef std::basic_string_view<CharT> string_view_t;
    string_view_t sv1 ( s1 );
    test1 ( sv1, pos1, n1, s2, expected );
}

void test0()
{
    test("", 0, 0, "", 0);
    test("", 0, 0, "abcde", -5);
    test("", 0, 0, "abcdefghij", -10);
    test("", 0, 0, "abcdefghijklmnopqrst", -20);
    test("", 0, 1, "", 0);
    test("", 0, 1, "abcde", -5);
    test("", 0, 1, "abcdefghij", -10);
    test("", 0, 1, "abcdefghijklmnopqrst", -20);
    test("", 1, 0, "", 0);
    test("", 1, 0, "abcde", 0);
    test("", 1, 0, "abcdefghij", 0);
    test("", 1, 0, "abcdefghijklmnopqrst", 0);
    test("abcde", 0, 0, "", 0);
    test("abcde", 0, 0, "abcde", -5);
    test("abcde", 0, 0, "abcdefghij", -10);
    test("abcde", 0, 0, "abcdefghijklmnopqrst", -20);
    test("abcde", 0, 1, "", 1);
    test("abcde", 0, 1, "abcde", -4);
    test("abcde", 0, 1, "abcdefghij", -9);
    test("abcde", 0, 1, "abcdefghijklmnopqrst", -19);
    test("abcde", 0, 2, "", 2);
    test("abcde", 0, 2, "abcde", -3);
    test("abcde", 0, 2, "abcdefghij", -8);
    test("abcde", 0, 2, "abcdefghijklmnopqrst", -18);
    test("abcde", 0, 4, "", 4);
    test("abcde", 0, 4, "abcde", -1);
    test("abcde", 0, 4, "abcdefghij", -6);
    test("abcde", 0, 4, "abcdefghijklmnopqrst", -16);
    test("abcde", 0, 5, "", 5);
    test("abcde", 0, 5, "abcde", 0);
    test("abcde", 0, 5, "abcdefghij", -5);
    test("abcde", 0, 5, "abcdefghijklmnopqrst", -15);
    test("abcde", 0, 6, "", 5);
    test("abcde", 0, 6, "abcde", 0);
    test("abcde", 0, 6, "abcdefghij", -5);
    test("abcde", 0, 6, "abcdefghijklmnopqrst", -15);
    test("abcde", 1, 0, "", 0);
    test("abcde", 1, 0, "abcde", -5);
    test("abcde", 1, 0, "abcdefghij", -10);
    test("abcde", 1, 0, "abcdefghijklmnopqrst", -20);
    test("abcde", 1, 1, "", 1);
    test("abcde", 1, 1, "abcde", 1);
    test("abcde", 1, 1, "abcdefghij", 1);
    test("abcde", 1, 1, "abcdefghijklmnopqrst", 1);
    test("abcde", 1, 2, "", 2);
    test("abcde", 1, 2, "abcde", 1);
    test("abcde", 1, 2, "abcdefghij", 1);
    test("abcde", 1, 2, "abcdefghijklmnopqrst", 1);
    test("abcde", 1, 3, "", 3);
    test("abcde", 1, 3, "abcde", 1);
    test("abcde", 1, 3, "abcdefghij", 1);
    test("abcde", 1, 3, "abcdefghijklmnopqrst", 1);
    test("abcde", 1, 4, "", 4);
    test("abcde", 1, 4, "abcde", 1);
    test("abcde", 1, 4, "abcdefghij", 1);
    test("abcde", 1, 4, "abcdefghijklmnopqrst", 1);
    test("abcde", 1, 5, "", 4);
    test("abcde", 1, 5, "abcde", 1);
    test("abcde", 1, 5, "abcdefghij", 1);
    test("abcde", 1, 5, "abcdefghijklmnopqrst", 1);
    test("abcde", 2, 0, "", 0);
    test("abcde", 2, 0, "abcde", -5);
    test("abcde", 2, 0, "abcdefghij", -10);
    test("abcde", 2, 0, "abcdefghijklmnopqrst", -20);
    test("abcde", 2, 1, "", 1);
    test("abcde", 2, 1, "abcde", 2);
    test("abcde", 2, 1, "abcdefghij", 2);
    test("abcde", 2, 1, "abcdefghijklmnopqrst", 2);
    test("abcde", 2, 2, "", 2);
    test("abcde", 2, 2, "abcde", 2);
    test("abcde", 2, 2, "abcdefghij", 2);
    test("abcde", 2, 2, "abcdefghijklmnopqrst", 2);
    test("abcde", 2, 3, "", 3);
    test("abcde", 2, 3, "abcde", 2);
    test("abcde", 2, 3, "abcdefghij", 2);
    test("abcde", 2, 3, "abcdefghijklmnopqrst", 2);
    test("abcde", 2, 4, "", 3);
    test("abcde", 2, 4, "abcde", 2);
    test("abcde", 2, 4, "abcdefghij", 2);
    test("abcde", 2, 4, "abcdefghijklmnopqrst", 2);
    test("abcde", 4, 0, "", 0);
    test("abcde", 4, 0, "abcde", -5);
    test("abcde", 4, 0, "abcdefghij", -10);
    test("abcde", 4, 0, "abcdefghijklmnopqrst", -20);
    test("abcde", 4, 1, "", 1);
    test("abcde", 4, 1, "abcde", 4);
    test("abcde", 4, 1, "abcdefghij", 4);
    test("abcde", 4, 1, "abcdefghijklmnopqrst", 4);
    test("abcde", 4, 2, "", 1);
    test("abcde", 4, 2, "abcde", 4);
    test("abcde", 4, 2, "abcdefghij", 4);
    test("abcde", 4, 2, "abcdefghijklmnopqrst", 4);
    test("abcde", 5, 0, "", 0);
    test("abcde", 5, 0, "abcde", -5);
    test("abcde", 5, 0, "abcdefghij", -10);
    test("abcde", 5, 0, "abcdefghijklmnopqrst", -20);
    test("abcde", 5, 1, "", 0);
    test("abcde", 5, 1, "abcde", -5);
    test("abcde", 5, 1, "abcdefghij", -10);
    test("abcde", 5, 1, "abcdefghijklmnopqrst", -20);
}

void test1()
{
    test("abcde", 6, 0, "", 0);
    test("abcde", 6, 0, "abcde", 0);
    test("abcde", 6, 0, "abcdefghij", 0);
    test("abcde", 6, 0, "abcdefghijklmnopqrst", 0);
    test("abcdefghij", 0, 0, "", 0);
    test("abcdefghij", 0, 0, "abcde", -5);
    test("abcdefghij", 0, 0, "abcdefghij", -10);
    test("abcdefghij", 0, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghij", 0, 1, "", 1);
    test("abcdefghij", 0, 1, "abcde", -4);
    test("abcdefghij", 0, 1, "abcdefghij", -9);
    test("abcdefghij", 0, 1, "abcdefghijklmnopqrst", -19);
    test("abcdefghij", 0, 5, "", 5);
    test("abcdefghij", 0, 5, "abcde", 0);
    test("abcdefghij", 0, 5, "abcdefghij", -5);
    test("abcdefghij", 0, 5, "abcdefghijklmnopqrst", -15);
    test("abcdefghij", 0, 9, "", 9);
    test("abcdefghij", 0, 9, "abcde", 4);
    test("abcdefghij", 0, 9, "abcdefghij", -1);
    test("abcdefghij", 0, 9, "abcdefghijklmnopqrst", -11);
    test("abcdefghij", 0, 10, "", 10);
    test("abcdefghij", 0, 10, "abcde", 5);
    test("abcdefghij", 0, 10, "abcdefghij", 0);
    test("abcdefghij", 0, 10, "abcdefghijklmnopqrst", -10);
    test("abcdefghij", 0, 11, "", 10);
    test("abcdefghij", 0, 11, "abcde", 5);
    test("abcdefghij", 0, 11, "abcdefghij", 0);
    test("abcdefghij", 0, 11, "abcdefghijklmnopqrst", -10);
    test("abcdefghij", 1, 0, "", 0);
    test("abcdefghij", 1, 0, "abcde", -5);
    test("abcdefghij", 1, 0, "abcdefghij", -10);
    test("abcdefghij", 1, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghij", 1, 1, "", 1);
    test("abcdefghij", 1, 1, "abcde", 1);
    test("abcdefghij", 1, 1, "abcdefghij", 1);
    test("abcdefghij", 1, 1, "abcdefghijklmnopqrst", 1);
    test("abcdefghij", 1, 4, "", 4);
    test("abcdefghij", 1, 4, "abcde", 1);
    test("abcdefghij", 1, 4, "abcdefghij", 1);
    test("abcdefghij", 1, 4, "abcdefghijklmnopqrst", 1);
    test("abcdefghij", 1, 8, "", 8);
    test("abcdefghij", 1, 8, "abcde", 1);
    test("abcdefghij", 1, 8, "abcdefghij", 1);
    test("abcdefghij", 1, 8, "abcdefghijklmnopqrst", 1);
    test("abcdefghij", 1, 9, "", 9);
    test("abcdefghij", 1, 9, "abcde", 1);
    test("abcdefghij", 1, 9, "abcdefghij", 1);
    test("abcdefghij", 1, 9, "abcdefghijklmnopqrst", 1);
    test("abcdefghij", 1, 10, "", 9);
    test("abcdefghij", 1, 10, "abcde", 1);
    test("abcdefghij", 1, 10, "abcdefghij", 1);
    test("abcdefghij", 1, 10, "abcdefghijklmnopqrst", 1);
    test("abcdefghij", 5, 0, "", 0);
    test("abcdefghij", 5, 0, "abcde", -5);
    test("abcdefghij", 5, 0, "abcdefghij", -10);
    test("abcdefghij", 5, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghij", 5, 1, "", 1);
    test("abcdefghij", 5, 1, "abcde", 5);
    test("abcdefghij", 5, 1, "abcdefghij", 5);
    test("abcdefghij", 5, 1, "abcdefghijklmnopqrst", 5);
    test("abcdefghij", 5, 2, "", 2);
    test("abcdefghij", 5, 2, "abcde", 5);
    test("abcdefghij", 5, 2, "abcdefghij", 5);
    test("abcdefghij", 5, 2, "abcdefghijklmnopqrst", 5);
    test("abcdefghij", 5, 4, "", 4);
    test("abcdefghij", 5, 4, "abcde", 5);
    test("abcdefghij", 5, 4, "abcdefghij", 5);
    test("abcdefghij", 5, 4, "abcdefghijklmnopqrst", 5);
    test("abcdefghij", 5, 5, "", 5);
    test("abcdefghij", 5, 5, "abcde", 5);
    test("abcdefghij", 5, 5, "abcdefghij", 5);
    test("abcdefghij", 5, 5, "abcdefghijklmnopqrst", 5);
    test("abcdefghij", 5, 6, "", 5);
    test("abcdefghij", 5, 6, "abcde", 5);
    test("abcdefghij", 5, 6, "abcdefghij", 5);
    test("abcdefghij", 5, 6, "abcdefghijklmnopqrst", 5);
    test("abcdefghij", 9, 0, "", 0);
    test("abcdefghij", 9, 0, "abcde", -5);
    test("abcdefghij", 9, 0, "abcdefghij", -10);
    test("abcdefghij", 9, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghij", 9, 1, "", 1);
    test("abcdefghij", 9, 1, "abcde", 9);
    test("abcdefghij", 9, 1, "abcdefghij", 9);
    test("abcdefghij", 9, 1, "abcdefghijklmnopqrst", 9);
    test("abcdefghij", 9, 2, "", 1);
    test("abcdefghij", 9, 2, "abcde", 9);
    test("abcdefghij", 9, 2, "abcdefghij", 9);
    test("abcdefghij", 9, 2, "abcdefghijklmnopqrst", 9);
    test("abcdefghij", 10, 0, "", 0);
    test("abcdefghij", 10, 0, "abcde", -5);
    test("abcdefghij", 10, 0, "abcdefghij", -10);
    test("abcdefghij", 10, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghij", 10, 1, "", 0);
    test("abcdefghij", 10, 1, "abcde", -5);
    test("abcdefghij", 10, 1, "abcdefghij", -10);
    test("abcdefghij", 10, 1, "abcdefghijklmnopqrst", -20);
    test("abcdefghij", 11, 0, "", 0);
    test("abcdefghij", 11, 0, "abcde", 0);
    test("abcdefghij", 11, 0, "abcdefghij", 0);
    test("abcdefghij", 11, 0, "abcdefghijklmnopqrst", 0);
}

void test2()
{
    test("abcdefghijklmnopqrst", 0, 0, "", 0);
    test("abcdefghijklmnopqrst", 0, 0, "abcde", -5);
    test("abcdefghijklmnopqrst", 0, 0, "abcdefghij", -10);
    test("abcdefghijklmnopqrst", 0, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghijklmnopqrst", 0, 1, "", 1);
    test("abcdefghijklmnopqrst", 0, 1, "abcde", -4);
    test("abcdefghijklmnopqrst", 0, 1, "abcdefghij", -9);
    test("abcdefghijklmnopqrst", 0, 1, "abcdefghijklmnopqrst", -19);
    test("abcdefghijklmnopqrst", 0, 10, "", 10);
    test("abcdefghijklmnopqrst", 0, 10, "abcde", 5);
    test("abcdefghijklmnopqrst", 0, 10, "abcdefghij", 0);
    test("abcdefghijklmnopqrst", 0, 10, "abcdefghijklmnopqrst", -10);
    test("abcdefghijklmnopqrst", 0, 19, "", 19);
    test("abcdefghijklmnopqrst", 0, 19, "abcde", 14);
    test("abcdefghijklmnopqrst", 0, 19, "abcdefghij", 9);
    test("abcdefghijklmnopqrst", 0, 19, "abcdefghijklmnopqrst", -1);
    test("abcdefghijklmnopqrst", 0, 20, "", 20);
    test("abcdefghijklmnopqrst", 0, 20, "abcde", 15);
    test("abcdefghijklmnopqrst", 0, 20, "abcdefghij", 10);
    test("abcdefghijklmnopqrst", 0, 20, "abcdefghijklmnopqrst", 0);
    test("abcdefghijklmnopqrst", 0, 21, "", 20);
    test("abcdefghijklmnopqrst", 0, 21, "abcde", 15);
    test("abcdefghijklmnopqrst", 0, 21, "abcdefghij", 10);
    test("abcdefghijklmnopqrst", 0, 21, "abcdefghijklmnopqrst", 0);
    test("abcdefghijklmnopqrst", 1, 0, "", 0);
    test("abcdefghijklmnopqrst", 1, 0, "abcde", -5);
    test("abcdefghijklmnopqrst", 1, 0, "abcdefghij", -10);
    test("abcdefghijklmnopqrst", 1, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghijklmnopqrst", 1, 1, "", 1);
    test("abcdefghijklmnopqrst", 1, 1, "abcde", 1);
    test("abcdefghijklmnopqrst", 1, 1, "abcdefghij", 1);
    test("abcdefghijklmnopqrst", 1, 1, "abcdefghijklmnopqrst", 1);
    test("abcdefghijklmnopqrst", 1, 9, "", 9);
    test("abcdefghijklmnopqrst", 1, 9, "abcde", 1);
    test("abcdefghijklmnopqrst", 1, 9, "abcdefghij", 1);
    test("abcdefghijklmnopqrst", 1, 9, "abcdefghijklmnopqrst", 1);
    test("abcdefghijklmnopqrst", 1, 18, "", 18);
    test("abcdefghijklmnopqrst", 1, 18, "abcde", 1);
    test("abcdefghijklmnopqrst", 1, 18, "abcdefghij", 1);
    test("abcdefghijklmnopqrst", 1, 18, "abcdefghijklmnopqrst", 1);
    test("abcdefghijklmnopqrst", 1, 19, "", 19);
    test("abcdefghijklmnopqrst", 1, 19, "abcde", 1);
    test("abcdefghijklmnopqrst", 1, 19, "abcdefghij", 1);
    test("abcdefghijklmnopqrst", 1, 19, "abcdefghijklmnopqrst", 1);
    test("abcdefghijklmnopqrst", 1, 20, "", 19);
    test("abcdefghijklmnopqrst", 1, 20, "abcde", 1);
    test("abcdefghijklmnopqrst", 1, 20, "abcdefghij", 1);
    test("abcdefghijklmnopqrst", 1, 20, "abcdefghijklmnopqrst", 1);
    test("abcdefghijklmnopqrst", 10, 0, "", 0);
    test("abcdefghijklmnopqrst", 10, 0, "abcde", -5);
    test("abcdefghijklmnopqrst", 10, 0, "abcdefghij", -10);
    test("abcdefghijklmnopqrst", 10, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghijklmnopqrst", 10, 1, "", 1);
    test("abcdefghijklmnopqrst", 10, 1, "abcde", 10);
    test("abcdefghijklmnopqrst", 10, 1, "abcdefghij", 10);
    test("abcdefghijklmnopqrst", 10, 1, "abcdefghijklmnopqrst", 10);
    test("abcdefghijklmnopqrst", 10, 5, "", 5);
    test("abcdefghijklmnopqrst", 10, 5, "abcde", 10);
    test("abcdefghijklmnopqrst", 10, 5, "abcdefghij", 10);
    test("abcdefghijklmnopqrst", 10, 5, "abcdefghijklmnopqrst", 10);
    test("abcdefghijklmnopqrst", 10, 9, "", 9);
    test("abcdefghijklmnopqrst", 10, 9, "abcde", 10);
    test("abcdefghijklmnopqrst", 10, 9, "abcdefghij", 10);
    test("abcdefghijklmnopqrst", 10, 9, "abcdefghijklmnopqrst", 10);
    test("abcdefghijklmnopqrst", 10, 10, "", 10);
    test("abcdefghijklmnopqrst", 10, 10, "abcde", 10);
    test("abcdefghijklmnopqrst", 10, 10, "abcdefghij", 10);
    test("abcdefghijklmnopqrst", 10, 10, "abcdefghijklmnopqrst", 10);
    test("abcdefghijklmnopqrst", 10, 11, "", 10);
    test("abcdefghijklmnopqrst", 10, 11, "abcde", 10);
    test("abcdefghijklmnopqrst", 10, 11, "abcdefghij", 10);
    test("abcdefghijklmnopqrst", 10, 11, "abcdefghijklmnopqrst", 10);
    test("abcdefghijklmnopqrst", 19, 0, "", 0);
    test("abcdefghijklmnopqrst", 19, 0, "abcde", -5);
    test("abcdefghijklmnopqrst", 19, 0, "abcdefghij", -10);
    test("abcdefghijklmnopqrst", 19, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghijklmnopqrst", 19, 1, "", 1);
    test("abcdefghijklmnopqrst", 19, 1, "abcde", 19);
    test("abcdefghijklmnopqrst", 19, 1, "abcdefghij", 19);
    test("abcdefghijklmnopqrst", 19, 1, "abcdefghijklmnopqrst", 19);
    test("abcdefghijklmnopqrst", 19, 2, "", 1);
    test("abcdefghijklmnopqrst", 19, 2, "abcde", 19);
    test("abcdefghijklmnopqrst", 19, 2, "abcdefghij", 19);
    test("abcdefghijklmnopqrst", 19, 2, "abcdefghijklmnopqrst", 19);
    test("abcdefghijklmnopqrst", 20, 0, "", 0);
    test("abcdefghijklmnopqrst", 20, 0, "abcde", -5);
    test("abcdefghijklmnopqrst", 20, 0, "abcdefghij", -10);
    test("abcdefghijklmnopqrst", 20, 0, "abcdefghijklmnopqrst", -20);
    test("abcdefghijklmnopqrst", 20, 1, "", 0);
    test("abcdefghijklmnopqrst", 20, 1, "abcde", -5);
    test("abcdefghijklmnopqrst", 20, 1, "abcdefghij", -10);
    test("abcdefghijklmnopqrst", 20, 1, "abcdefghijklmnopqrst", -20);
    test("abcdefghijklmnopqrst", 21, 0, "", 0);
    test("abcdefghijklmnopqrst", 21, 0, "abcde", 0);
    test("abcdefghijklmnopqrst", 21, 0, "abcdefghij", 0);
    test("abcdefghijklmnopqrst", 21, 0, "abcdefghijklmnopqrst", 0);
}


int main()
{
    test0();
    test1();
    test2();

    {
    test("", 0, 0, "", 0);
    test("", 0, 0, "abcde", -5);
    test("", 0, 0, "abcdefghij", -10);
    test("", 0, 0, "abcdefghijklmnopqrst", -20);
    test("abcde", 0, 2, "", 5);
    test("abcde", 0, 6,"abcde", 0);
    test("abcde", 0, 6, "abcdefghij", -5);
    test("abcde", 0, 6, "abcdefghijklmnopqrst", -15);
    test("abcdefghij", 3, 3, "", 10);
    test("abcdefghij", 3, 3,"abcde", 5);
    test("abcdefghij", 3, 3, "def", 0);
    test("abcdefghij", 0, 4, "abcdefghijklmnopqrst", -10);
    test("abcdefghijklmnopqrst", 5, 5, "", 20);
    test("abcdefghijklmnopqrst", 0, 8, "abcde", 15);
    test("abcdefghijklmnopqrst", 0, 12, "abcdefghij", 10);
    test("abcdefghijklmnopqrst", 0, static_cast<size_t>(-1), "abcdefghijklmnopqrst", 0);
    }

    {
    test(L"", 0, 0, L"", 0);
    test(L"", 0, 0, L"abcde", -5);
    test(L"", 0, 0, L"abcdefghij", -10);
    test(L"", 0, 0, L"abcdefghijklmnopqrst", -20);
    test(L"abcde", 0, 2, L"", 5);
    test(L"abcde", 0, 6, L"abcde", 0);
    test(L"abcde", 0, 6, L"abcdefghij", -5);
    test(L"abcde", 0, 6, L"abcdefghijklmnopqrst", -15);
    test(L"abcdefghij", 3, 3, L"", 10);
    test(L"abcdefghij", 3, 3, L"abcde", 5);
    test(L"abcdefghij", 3, 3, L"def", 0);
    test(L"abcdefghij", 0, 4, L"abcdefghijklmnopqrst", -10);
    test(L"abcdefghijklmnopqrst", 5, 5, L"", 20);
    test(L"abcdefghijklmnopqrst", 0, 8, L"abcde", 15);
    test(L"abcdefghijklmnopqrst", 0, 12, L"abcdefghij", 10);
    test(L"abcdefghijklmnopqrst", 0, static_cast<size_t>(-1), L"abcdefghijklmnopqrst", 0);
    }

#if TEST_STD_VER >= 11
    {
    test(U"", 0, 0, U"", 0);
    test(U"", 0, 0, U"abcde", -5);
    test(U"", 0, 0, U"abcdefghij", -10);
    test(U"", 0, 0, U"abcdefghijklmnopqrst", -20);
    test(U"abcde", 0, 2, U"", 5);
    test(U"abcde", 0, 6, U"abcde", 0);
    test(U"abcde", 0, 6, U"abcdefghij", -5);
    test(U"abcde", 0, 6, U"abcdefghijklmnopqrst", -15);
    test(U"abcdefghij", 3, 3, U"", 10);
    test(U"abcdefghij", 3, 3, U"abcde", 5);
    test(U"abcdefghij", 3, 3, U"def", 0);
    test(U"abcdefghij", 0, 4, U"abcdefghijklmnopqrst", -10);
    test(U"abcdefghijklmnopqrst", 5, 5, U"", 20);
    test(U"abcdefghijklmnopqrst", 0, 8, U"abcde", 15);
    test(U"abcdefghijklmnopqrst", 0, 12, U"abcdefghij", 10);
    test(U"abcdefghijklmnopqrst", 0, static_cast<size_t>(-1), U"abcdefghijklmnopqrst", 0);
    }

    {
    test(u"", 0, 0, u"", 0);
    test(u"", 0, 0, u"abcde", -5);
    test(u"", 0, 0, u"abcdefghij", -10);
    test(u"", 0, 0, u"abcdefghijklmnopqrst", -20);
    test(u"abcde", 0, 2, u"", 5);
    test(u"abcde", 0, 6, u"abcde", 0);
    test(u"abcde", 0, 6, u"abcdefghij", -5);
    test(u"abcde", 0, 6, u"abcdefghijklmnopqrst", -15);
    test(u"abcdefghij", 3, 3, u"", 10);
    test(u"abcdefghij", 3, 3, u"abcde", 5);
    test(u"abcdefghij", 3, 3, u"def", 0);
    test(u"abcdefghij", 0, 4, u"abcdefghijklmnopqrst", -10);
    test(u"abcdefghijklmnopqrst", 5, 5, u"", 20);
    test(u"abcdefghijklmnopqrst", 0, 8, u"abcde", 15);
    test(u"abcdefghijklmnopqrst", 0, 12, u"abcdefghij", 10);
    test(u"abcdefghijklmnopqrst", 0, static_cast<size_t>(-1), u"abcdefghijklmnopqrst", 0);
    }
#endif

#if TEST_STD_VER > 11
    {
    typedef std::basic_string_view<char, constexpr_char_traits<char>> SV;
    constexpr SV  sv1;
    constexpr SV  sv2 { "abcde", 5 };
    static_assert ( sv1.compare(0, 0, "") == 0, "" );
    static_assert ( sv1.compare(0, 0, "abcde") == -1, "" );
    static_assert ( sv2.compare(0, 2, "") == 1, "" );
    static_assert ( sv2.compare(0, 6, "abcde") == 0, "" );
    }
#endif
}
