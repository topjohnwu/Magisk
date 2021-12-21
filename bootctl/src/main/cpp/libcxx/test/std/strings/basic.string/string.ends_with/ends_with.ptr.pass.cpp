//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <string>

//   bool ends_with(const CharT *x) const;

#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    typedef std::string S;
    const char *s = "abcde";

    S   s0;
    S   s1  { s + 4, 1 };
    S   s2  { s + 3, 2 };
//  S   s3  { s + 2, 3 };
//  S   s4  { s + 1, 4 };
//  S   s5  { s,     5 };
    S  sNot { "def", 3 };

    LIBCPP_ASSERT_NOEXCEPT(s0.ends_with(""));

    assert ( s0.ends_with(""));
    assert (!s0.ends_with("e"));

    assert ( s1.ends_with(""));
    assert ( s1.ends_with("e"));
    assert (!s1.ends_with("de"));
    assert (!s1.ends_with("cde"));
    assert (!s1.ends_with("bcde"));
    assert (!s1.ends_with("abcde"));
    assert (!s1.ends_with("def"));

    assert ( s2.ends_with(""));
    assert ( s2.ends_with("e"));
    assert ( s2.ends_with("de"));
    assert (!s2.ends_with("cde"));
    assert (!s2.ends_with("bcde"));
    assert (!s2.ends_with("abcde"));
    assert (!s2.ends_with("def"));

    assert ( sNot.ends_with(""));
    assert (!sNot.ends_with("e"));
    assert (!sNot.ends_with("de"));
    assert (!sNot.ends_with("cde"));
    assert (!sNot.ends_with("bcde"));
    assert (!sNot.ends_with("abcde"));
    assert ( sNot.ends_with("def"));
    }
}
