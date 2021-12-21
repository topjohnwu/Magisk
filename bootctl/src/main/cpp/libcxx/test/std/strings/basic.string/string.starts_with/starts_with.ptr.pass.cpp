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

//   bool starts_with(const CharT *x) const;

#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    typedef std::string S;
    const char *s = "abcde";
    S  s0 {};
    S  s1 { s, 1 };
    S  s2 { s, 2 };
//     S  s3 { s, 3 };
//     S  s4 { s, 4 };
//     S  s5 { s, 5 };
    S  sNot {"def", 3 };

    LIBCPP_ASSERT_NOEXCEPT(s0.starts_with(""));

    assert ( s0.starts_with(""));
    assert (!s0.starts_with("a"));

    assert ( s1.starts_with(""));
    assert ( s1.starts_with("a"));
    assert (!s1.starts_with("ab"));
    assert (!s1.starts_with("abc"));
    assert (!s1.starts_with("abcd"));
    assert (!s1.starts_with("abcde"));
    assert (!s1.starts_with("def"));

    assert ( s2.starts_with(""));
    assert ( s2.starts_with("a"));
    assert ( s2.starts_with("ab"));
    assert (!s2.starts_with("abc"));
    assert (!s2.starts_with("abcd"));
    assert (!s2.starts_with("abcde"));
    assert (!s2.starts_with("def"));

    assert ( sNot.starts_with(""));
    assert (!sNot.starts_with("a"));
    assert (!sNot.starts_with("ab"));
    assert (!sNot.starts_with("abc"));
    assert (!sNot.starts_with("abcd"));
    assert (!sNot.starts_with("abcde"));
    assert ( sNot.starts_with("def"));
    }
}
