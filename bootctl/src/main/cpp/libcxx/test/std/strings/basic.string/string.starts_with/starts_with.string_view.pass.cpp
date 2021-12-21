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

//   bool starts_with(string_view x) const noexcept;

#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    typedef std::string S;
    typedef std::string_view SV;
    const char *s = "abcde";

    S   s0;
    S   s1  { s, 1 };
    S   s2  { s, 2 };
//  S   s3  { s, 3 };
//  S   s4  { s, 4 };
//  S   s5  { s, 5 };
    S  sNot { "def", 3 };

    SV  sv0;
    SV  sv1 { s, 1 };
    SV  sv2 { s, 2 };
    SV  sv3 { s, 3 };
    SV  sv4 { s, 4 };
    SV  sv5 { s, 5 };
    SV svNot {"def", 3 };

    ASSERT_NOEXCEPT(s0.starts_with(sv0));

    assert ( s0.starts_with(sv0));
    assert (!s0.starts_with(sv1));

    assert ( s1.starts_with(sv0));
    assert ( s1.starts_with(sv1));
    assert (!s1.starts_with(sv2));
    assert (!s1.starts_with(sv3));
    assert (!s1.starts_with(sv4));
    assert (!s1.starts_with(sv5));
    assert (!s1.starts_with(svNot));

    assert ( s2.starts_with(sv0));
    assert ( s2.starts_with(sv1));
    assert ( s2.starts_with(sv2));
    assert (!s2.starts_with(sv3));
    assert (!s2.starts_with(sv4));
    assert (!s2.starts_with(sv5));
    assert (!s2.starts_with(svNot));

    assert ( sNot.starts_with(sv0));
    assert (!sNot.starts_with(sv1));
    assert (!sNot.starts_with(sv2));
    assert (!sNot.starts_with(sv3));
    assert (!sNot.starts_with(sv4));
    assert (!sNot.starts_with(sv5));
    assert ( sNot.starts_with(svNot));
    }
}
