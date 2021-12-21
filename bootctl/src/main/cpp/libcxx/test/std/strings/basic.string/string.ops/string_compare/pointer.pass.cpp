//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// int compare(const charT *s) const;

#include <string>
#include <cassert>

#include "min_allocator.h"

int sign(int x)
{
    if (x == 0)
        return 0;
    if (x < 0)
        return -1;
    return 1;
}

template <class S>
void
test(const S& s, const typename S::value_type* str, int x)
{
    assert(sign(s.compare(str)) == sign(x));
}

int main()
{
    {
    typedef std::string S;
    test(S(""), "", 0);
    test(S(""), "abcde", -5);
    test(S(""), "abcdefghij", -10);
    test(S(""), "abcdefghijklmnopqrst", -20);
    test(S("abcde"), "", 5);
    test(S("abcde"), "abcde", 0);
    test(S("abcde"), "abcdefghij", -5);
    test(S("abcde"), "abcdefghijklmnopqrst", -15);
    test(S("abcdefghij"), "", 10);
    test(S("abcdefghij"), "abcde", 5);
    test(S("abcdefghij"), "abcdefghij", 0);
    test(S("abcdefghij"), "abcdefghijklmnopqrst", -10);
    test(S("abcdefghijklmnopqrst"), "", 20);
    test(S("abcdefghijklmnopqrst"), "abcde", 15);
    test(S("abcdefghijklmnopqrst"), "abcdefghij", 10);
    test(S("abcdefghijklmnopqrst"), "abcdefghijklmnopqrst", 0);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), "", 0);
    test(S(""), "abcde", -5);
    test(S(""), "abcdefghij", -10);
    test(S(""), "abcdefghijklmnopqrst", -20);
    test(S("abcde"), "", 5);
    test(S("abcde"), "abcde", 0);
    test(S("abcde"), "abcdefghij", -5);
    test(S("abcde"), "abcdefghijklmnopqrst", -15);
    test(S("abcdefghij"), "", 10);
    test(S("abcdefghij"), "abcde", 5);
    test(S("abcdefghij"), "abcdefghij", 0);
    test(S("abcdefghij"), "abcdefghijklmnopqrst", -10);
    test(S("abcdefghijklmnopqrst"), "", 20);
    test(S("abcdefghijklmnopqrst"), "abcde", 15);
    test(S("abcdefghijklmnopqrst"), "abcdefghij", 10);
    test(S("abcdefghijklmnopqrst"), "abcdefghijklmnopqrst", 0);
    }
#endif
}
