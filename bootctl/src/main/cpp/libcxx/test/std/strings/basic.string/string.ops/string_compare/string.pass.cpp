//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// int compare(const basic_string& str) const

#include <string>
#include <cassert>

#include "test_macros.h"
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
test(const S& s, const S& str, int x)
{
    assert(sign(s.compare(str)) == sign(x));
}

int main()
{
    {
    typedef std::string S;
    test(S(""), S(""), 0);
    test(S(""), S("abcde"), -5);
    test(S(""), S("abcdefghij"), -10);
    test(S(""), S("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), S(""), 5);
    test(S("abcde"), S("abcde"), 0);
    test(S("abcde"), S("abcdefghij"), -5);
    test(S("abcde"), S("abcdefghijklmnopqrst"), -15);
    test(S("abcdefghij"), S(""), 10);
    test(S("abcdefghij"), S("abcde"), 5);
    test(S("abcdefghij"), S("abcdefghij"), 0);
    test(S("abcdefghij"), S("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghijklmnopqrst"), S(""), 20);
    test(S("abcdefghijklmnopqrst"), S("abcde"), 15);
    test(S("abcdefghijklmnopqrst"), S("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), S("abcdefghijklmnopqrst"), 0);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), S(""), 0);
    test(S(""), S("abcde"), -5);
    test(S(""), S("abcdefghij"), -10);
    test(S(""), S("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), S(""), 5);
    test(S("abcde"), S("abcde"), 0);
    test(S("abcde"), S("abcdefghij"), -5);
    test(S("abcde"), S("abcdefghijklmnopqrst"), -15);
    test(S("abcdefghij"), S(""), 10);
    test(S("abcdefghij"), S("abcde"), 5);
    test(S("abcdefghij"), S("abcdefghij"), 0);
    test(S("abcdefghij"), S("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghijklmnopqrst"), S(""), 20);
    test(S("abcdefghijklmnopqrst"), S("abcde"), 15);
    test(S("abcdefghijklmnopqrst"), S("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), S("abcdefghijklmnopqrst"), 0);
    }
#endif

#if TEST_STD_VER > 3
    {   // LWG 2946
    std::string s = " !";
    assert(s.compare({"abc", 1}) < 0);
    }
#endif
}
