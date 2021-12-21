//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// int compare(const basic_string_view sv) const

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

template <class S, class SV>
void
test(const S& s, SV sv, int x)
{
    assert(sign(s.compare(sv)) == sign(x));
}

int main()
{
    {
    typedef std::string S;
    typedef std::string_view SV;
    test(S(""), SV(""), 0);
    test(S(""), SV("abcde"), -5);
    test(S(""), SV("abcdefghij"), -10);
    test(S(""), SV("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), SV(""), 5);
    test(S("abcde"), SV("abcde"), 0);
    test(S("abcde"), SV("abcdefghij"), -5);
    test(S("abcde"), SV("abcdefghijklmnopqrst"), -15);
    test(S("abcdefghij"), SV(""), 10);
    test(S("abcdefghij"), SV("abcde"), 5);
    test(S("abcdefghij"), SV("abcdefghij"), 0);
    test(S("abcdefghij"), SV("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghijklmnopqrst"), SV(""), 20);
    test(S("abcdefghijklmnopqrst"), SV("abcde"), 15);
    test(S("abcdefghijklmnopqrst"), SV("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), SV("abcdefghijklmnopqrst"), 0);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    typedef std::string_view SV;
    test(S(""), SV(""), 0);
    test(S(""), SV("abcde"), -5);
    test(S(""), SV("abcdefghij"), -10);
    test(S(""), SV("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), SV(""), 5);
    test(S("abcde"), SV("abcde"), 0);
    test(S("abcde"), SV("abcdefghij"), -5);
    test(S("abcde"), SV("abcdefghijklmnopqrst"), -15);
    test(S("abcdefghij"), SV(""), 10);
    test(S("abcdefghij"), SV("abcde"), 5);
    test(S("abcdefghij"), SV("abcdefghij"), 0);
    test(S("abcdefghij"), SV("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghijklmnopqrst"), SV(""), 20);
    test(S("abcdefghijklmnopqrst"), SV("abcde"), 15);
    test(S("abcdefghijklmnopqrst"), SV("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), SV("abcdefghijklmnopqrst"), 0);
    }
#endif
}
