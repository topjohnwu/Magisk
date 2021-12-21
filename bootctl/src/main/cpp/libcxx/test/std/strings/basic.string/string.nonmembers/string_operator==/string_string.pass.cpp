//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<class charT, class traits, class Allocator>
//   bool operator==(const basic_string<charT,traits,Allocator>& lhs,
//                   const basic_string<charT,traits,Allocator>& rhs);

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(const S& lhs, const S& rhs, bool x)
{
    assert((lhs == rhs) == x);
}

int main()
{
    {
    typedef std::string S;
    test(S(""), S(""), true);
    test(S(""), S("abcde"), false);
    test(S(""), S("abcdefghij"), false);
    test(S(""), S("abcdefghijklmnopqrst"), false);
    test(S("abcde"), S(""), false);
    test(S("abcde"), S("abcde"), true);
    test(S("abcde"), S("abcdefghij"), false);
    test(S("abcde"), S("abcdefghijklmnopqrst"), false);
    test(S("abcdefghij"), S(""), false);
    test(S("abcdefghij"), S("abcde"), false);
    test(S("abcdefghij"), S("abcdefghij"), true);
    test(S("abcdefghij"), S("abcdefghijklmnopqrst"), false);
    test(S("abcdefghijklmnopqrst"), S(""), false);
    test(S("abcdefghijklmnopqrst"), S("abcde"), false);
    test(S("abcdefghijklmnopqrst"), S("abcdefghij"), false);
    test(S("abcdefghijklmnopqrst"), S("abcdefghijklmnopqrst"), true);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), S(""), true);
    test(S(""), S("abcde"), false);
    test(S(""), S("abcdefghij"), false);
    test(S(""), S("abcdefghijklmnopqrst"), false);
    test(S("abcde"), S(""), false);
    test(S("abcde"), S("abcde"), true);
    test(S("abcde"), S("abcdefghij"), false);
    test(S("abcde"), S("abcdefghijklmnopqrst"), false);
    test(S("abcdefghij"), S(""), false);
    test(S("abcdefghij"), S("abcde"), false);
    test(S("abcdefghij"), S("abcdefghij"), true);
    test(S("abcdefghij"), S("abcdefghijklmnopqrst"), false);
    test(S("abcdefghijklmnopqrst"), S(""), false);
    test(S("abcdefghijklmnopqrst"), S("abcde"), false);
    test(S("abcdefghijklmnopqrst"), S("abcdefghij"), false);
    test(S("abcdefghijklmnopqrst"), S("abcdefghijklmnopqrst"), true);
    }
#endif
}
