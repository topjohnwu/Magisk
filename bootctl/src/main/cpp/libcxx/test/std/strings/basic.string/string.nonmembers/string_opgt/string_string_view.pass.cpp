//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// we get this comparison "for free" because the string implicitly converts to the string_view

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S, class SV>
void
test(const S& lhs, SV rhs, bool x)
{
    assert((lhs > rhs) == x);
}

int main()
{
    {
    typedef std::string S;
    typedef std::string_view SV;
    test(S(""), SV(""), false);
    test(S(""), SV("abcde"), false);
    test(S(""), SV("abcdefghij"), false);
    test(S(""), SV("abcdefghijklmnopqrst"), false);
    test(S("abcde"), SV(""), true);
    test(S("abcde"), SV("abcde"), false);
    test(S("abcde"), SV("abcdefghij"), false);
    test(S("abcde"), SV("abcdefghijklmnopqrst"), false);
    test(S("abcdefghij"), SV(""), true);
    test(S("abcdefghij"), SV("abcde"), true);
    test(S("abcdefghij"), SV("abcdefghij"), false);
    test(S("abcdefghij"), SV("abcdefghijklmnopqrst"), false);
    test(S("abcdefghijklmnopqrst"), SV(""), true);
    test(S("abcdefghijklmnopqrst"), SV("abcde"), true);
    test(S("abcdefghijklmnopqrst"), SV("abcdefghij"), true);
    test(S("abcdefghijklmnopqrst"), SV("abcdefghijklmnopqrst"), false);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string     <char, std::char_traits<char>, min_allocator<char>> S;
    typedef std::basic_string_view<char, std::char_traits<char>> SV;
    test(S(""), SV(""), false);
    test(S(""), SV("abcde"), false);
    test(S(""), SV("abcdefghij"), false);
    test(S(""), SV("abcdefghijklmnopqrst"), false);
    test(S("abcde"), SV(""), true);
    test(S("abcde"), SV("abcde"), false);
    test(S("abcde"), SV("abcdefghij"), false);
    test(S("abcde"), SV("abcdefghijklmnopqrst"), false);
    test(S("abcdefghij"), SV(""), true);
    test(S("abcdefghij"), SV("abcde"), true);
    test(S("abcdefghij"), SV("abcdefghij"), false);
    test(S("abcdefghij"), SV("abcdefghijklmnopqrst"), false);
    test(S("abcdefghijklmnopqrst"), SV(""), true);
    test(S("abcdefghijklmnopqrst"), SV("abcde"), true);
    test(S("abcdefghijklmnopqrst"), SV("abcdefghij"), true);
    test(S("abcdefghijklmnopqrst"), SV("abcdefghijklmnopqrst"), false);
    }
#endif
}
