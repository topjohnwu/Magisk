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
//   bool operator>=(const charT* lhs, const basic_string<charT,traits,Allocator>& rhs);

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(const typename S::value_type* lhs, const S& rhs, bool x)
{
    assert((lhs >= rhs) == x);
}

int main()
{
    {
    typedef std::string S;
    test("", S(""), true);
    test("", S("abcde"), false);
    test("", S("abcdefghij"), false);
    test("", S("abcdefghijklmnopqrst"), false);
    test("abcde", S(""), true);
    test("abcde", S("abcde"), true);
    test("abcde", S("abcdefghij"), false);
    test("abcde", S("abcdefghijklmnopqrst"), false);
    test("abcdefghij", S(""), true);
    test("abcdefghij", S("abcde"), true);
    test("abcdefghij", S("abcdefghij"), true);
    test("abcdefghij", S("abcdefghijklmnopqrst"), false);
    test("abcdefghijklmnopqrst", S(""), true);
    test("abcdefghijklmnopqrst", S("abcde"), true);
    test("abcdefghijklmnopqrst", S("abcdefghij"), true);
    test("abcdefghijklmnopqrst", S("abcdefghijklmnopqrst"), true);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test("", S(""), true);
    test("", S("abcde"), false);
    test("", S("abcdefghij"), false);
    test("", S("abcdefghijklmnopqrst"), false);
    test("abcde", S(""), true);
    test("abcde", S("abcde"), true);
    test("abcde", S("abcdefghij"), false);
    test("abcde", S("abcdefghijklmnopqrst"), false);
    test("abcdefghij", S(""), true);
    test("abcdefghij", S("abcde"), true);
    test("abcdefghij", S("abcdefghij"), true);
    test("abcdefghij", S("abcdefghijklmnopqrst"), false);
    test("abcdefghijklmnopqrst", S(""), true);
    test("abcdefghijklmnopqrst", S("abcde"), true);
    test("abcdefghijklmnopqrst", S("abcdefghij"), true);
    test("abcdefghijklmnopqrst", S("abcdefghijklmnopqrst"), true);
    }
#endif
}
