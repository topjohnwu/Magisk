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

//   bool ends_with(basic_string_view x) const noexcept;

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
    S   s1  { s + 4, 1 };
    S   s2  { s + 3, 2 };
//  S   s3  { s + 2, 3 };
//  S   s4  { s + 1, 4 };
//  S   s5  { s,     5 };
    S  sNot { "def", 3 };

    SV  sv0;
    SV  sv1 { s + 4, 1 };
    SV  sv2 { s + 3, 2 };
    SV  sv3 { s + 2, 3 };
    SV  sv4 { s + 1, 4 };
    SV  sv5 { s    , 5 };
    SV svNot {"def", 3 };

    ASSERT_NOEXCEPT(s0.ends_with(sv0));

    assert ( s0.ends_with(sv0));
    assert (!s0.ends_with(sv1));

    assert ( s1.ends_with(sv0));
    assert ( s1.ends_with(sv1));
    assert (!s1.ends_with(sv2));
    assert (!s1.ends_with(sv3));
    assert (!s1.ends_with(sv4));
    assert (!s1.ends_with(sv5));
    assert (!s1.ends_with(svNot));

    assert ( s2.ends_with(sv0));
    assert ( s2.ends_with(sv1));
    assert ( s2.ends_with(sv2));
    assert (!s2.ends_with(sv3));
    assert (!s2.ends_with(sv4));
    assert (!s2.ends_with(sv5));
    assert (!s2.ends_with(svNot));

    assert ( sNot.ends_with(sv0));
    assert (!sNot.ends_with(sv1));
    assert (!sNot.ends_with(sv2));
    assert (!sNot.ends_with(sv3));
    assert (!sNot.ends_with(sv4));
    assert (!sNot.ends_with(sv5));
    assert ( sNot.ends_with(svNot));
    }
}
