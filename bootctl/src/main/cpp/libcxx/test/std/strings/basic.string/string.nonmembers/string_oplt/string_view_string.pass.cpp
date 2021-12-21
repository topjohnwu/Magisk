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
test(SV lhs, const S& rhs, bool x)
{
    assert((lhs < rhs) == x);
}

int main()
{
    {
    typedef std::string S;
    typedef std::string_view SV;
    test(SV(""), S(""), false);
    test(SV(""), S("abcde"), true);
    test(SV(""), S("abcdefghij"), true);
    test(SV(""), S("abcdefghijklmnopqrst"), true);
    test(SV("abcde"), S(""), false);
    test(SV("abcde"), S("abcde"), false);
    test(SV("abcde"), S("abcdefghij"), true);
    test(SV("abcde"), S("abcdefghijklmnopqrst"), true);
    test(SV("abcdefghij"), S(""), false);
    test(SV("abcdefghij"), S("abcde"), false);
    test(SV("abcdefghij"), S("abcdefghij"), false);
    test(SV("abcdefghij"), S("abcdefghijklmnopqrst"), true);
    test(SV("abcdefghijklmnopqrst"), S(""), false);
    test(SV("abcdefghijklmnopqrst"), S("abcde"), false);
    test(SV("abcdefghijklmnopqrst"), S("abcdefghij"), false);
    test(SV("abcdefghijklmnopqrst"), S("abcdefghijklmnopqrst"), false);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    typedef std::basic_string_view<char, std::char_traits<char>> SV;
    test(SV(""), S(""), false);
    test(SV(""), S("abcde"), true);
    test(SV(""), S("abcdefghij"), true);
    test(SV(""), S("abcdefghijklmnopqrst"), true);
    test(SV("abcde"), S(""), false);
    test(SV("abcde"), S("abcde"), false);
    test(SV("abcde"), S("abcdefghij"), true);
    test(SV("abcde"), S("abcdefghijklmnopqrst"), true);
    test(SV("abcdefghij"), S(""), false);
    test(SV("abcdefghij"), S("abcde"), false);
    test(SV("abcdefghij"), S("abcdefghij"), false);
    test(SV("abcdefghij"), S("abcdefghijklmnopqrst"), true);
    test(SV("abcdefghijklmnopqrst"), S(""), false);
    test(SV("abcdefghijklmnopqrst"), S("abcde"), false);
    test(SV("abcdefghijklmnopqrst"), S("abcdefghij"), false);
    test(SV("abcdefghijklmnopqrst"), S("abcdefghijklmnopqrst"), false);
    }
#endif
}
