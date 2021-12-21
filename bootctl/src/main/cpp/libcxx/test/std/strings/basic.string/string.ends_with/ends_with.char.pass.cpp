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

//   bool ends_with(charT x) const noexcept;

#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    typedef std::string S;
    S  s1 {};
    S  s2 { "abcde", 5 };

    ASSERT_NOEXCEPT(s1.ends_with('e'));

    assert (!s1.ends_with('e'));
    assert (!s1.ends_with('x'));
    assert ( s2.ends_with('e'));
    assert (!s2.ends_with('x'));
    }
}
