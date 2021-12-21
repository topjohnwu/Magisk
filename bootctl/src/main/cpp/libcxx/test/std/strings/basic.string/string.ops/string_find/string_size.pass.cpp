//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type find(const basic_string& str, size_type pos = 0) const;

#include <string>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(const S& s, const S& str, typename S::size_type pos, typename S::size_type x)
{
    assert(s.find(str, pos) == x);
    if (x != S::npos)
        assert(pos <= x && x + str.size() <= s.size());
}

template <class S>
void
test(const S& s, const S& str, typename S::size_type x)
{
    assert(s.find(str) == x);
    if (x != S::npos)
        assert(0 <= x && x + str.size() <= s.size());
}

template <class S>
void test0()
{
    test(S(""), S(""), 0, 0);
    test(S(""), S("abcde"), 0, S::npos);
    test(S(""), S("abcdeabcde"), 0, S::npos);
    test(S(""), S("abcdeabcdeabcdeabcde"), 0, S::npos);
    test(S(""), S(""), 1, S::npos);
    test(S(""), S("abcde"), 1, S::npos);
    test(S(""), S("abcdeabcde"), 1, S::npos);
    test(S(""), S("abcdeabcdeabcdeabcde"), 1, S::npos);
    test(S("abcde"), S(""), 0, 0);
    test(S("abcde"), S("abcde"), 0, 0);
    test(S("abcde"), S("abcdeabcde"), 0, S::npos);
    test(S("abcde"), S("abcdeabcdeabcdeabcde"), 0, S::npos);
    test(S("abcde"), S(""), 1, 1);
    test(S("abcde"), S("abcde"), 1, S::npos);
    test(S("abcde"), S("abcdeabcde"), 1, S::npos);
    test(S("abcde"), S("abcdeabcdeabcdeabcde"), 1, S::npos);
    test(S("abcde"), S(""), 2, 2);
    test(S("abcde"), S("abcde"), 2, S::npos);
    test(S("abcde"), S("abcdeabcde"), 2, S::npos);
    test(S("abcde"), S("abcdeabcdeabcdeabcde"), 2, S::npos);
    test(S("abcde"), S(""), 4, 4);
    test(S("abcde"), S("abcde"), 4, S::npos);
    test(S("abcde"), S("abcdeabcde"), 4, S::npos);
    test(S("abcde"), S("abcdeabcdeabcdeabcde"), 4, S::npos);
    test(S("abcde"), S(""), 5, 5);
    test(S("abcde"), S("abcde"), 5, S::npos);
    test(S("abcde"), S("abcdeabcde"), 5, S::npos);
    test(S("abcde"), S("abcdeabcdeabcdeabcde"), 5, S::npos);
    test(S("abcde"), S(""), 6, S::npos);
    test(S("abcde"), S("abcde"), 6, S::npos);
    test(S("abcde"), S("abcdeabcde"), 6, S::npos);
    test(S("abcde"), S("abcdeabcdeabcdeabcde"), 6, S::npos);
    test(S("abcdeabcde"), S(""), 0, 0);
    test(S("abcdeabcde"), S("abcde"), 0, 0);
    test(S("abcdeabcde"), S("abcdeabcde"), 0, 0);
    test(S("abcdeabcde"), S("abcdeabcdeabcdeabcde"), 0, S::npos);
    test(S("abcdeabcde"), S(""), 1, 1);
    test(S("abcdeabcde"), S("abcde"), 1, 5);
    test(S("abcdeabcde"), S("abcdeabcde"), 1, S::npos);
    test(S("abcdeabcde"), S("abcdeabcdeabcdeabcde"), 1, S::npos);
    test(S("abcdeabcde"), S(""), 5, 5);
    test(S("abcdeabcde"), S("abcde"), 5, 5);
    test(S("abcdeabcde"), S("abcdeabcde"), 5, S::npos);
    test(S("abcdeabcde"), S("abcdeabcdeabcdeabcde"), 5, S::npos);
    test(S("abcdeabcde"), S(""), 9, 9);
    test(S("abcdeabcde"), S("abcde"), 9, S::npos);
    test(S("abcdeabcde"), S("abcdeabcde"), 9, S::npos);
    test(S("abcdeabcde"), S("abcdeabcdeabcdeabcde"), 9, S::npos);
    test(S("abcdeabcde"), S(""), 10, 10);
    test(S("abcdeabcde"), S("abcde"), 10, S::npos);
    test(S("abcdeabcde"), S("abcdeabcde"), 10, S::npos);
    test(S("abcdeabcde"), S("abcdeabcdeabcdeabcde"), 10, S::npos);
    test(S("abcdeabcde"), S(""), 11, S::npos);
    test(S("abcdeabcde"), S("abcde"), 11, S::npos);
    test(S("abcdeabcde"), S("abcdeabcde"), 11, S::npos);
    test(S("abcdeabcde"), S("abcdeabcdeabcdeabcde"), 11, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S(""), 0, 0);
    test(S("abcdeabcdeabcdeabcde"), S("abcde"), 0, 0);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcde"), 0, 0);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcdeabcdeabcde"), 0, 0);
    test(S("abcdeabcdeabcdeabcde"), S(""), 1, 1);
    test(S("abcdeabcdeabcdeabcde"), S("abcde"), 1, 5);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcde"), 1, 5);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcdeabcdeabcde"), 1, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S(""), 10, 10);
    test(S("abcdeabcdeabcdeabcde"), S("abcde"), 10, 10);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcde"), 10, 10);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcdeabcdeabcde"), 10, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S(""), 19, 19);
    test(S("abcdeabcdeabcdeabcde"), S("abcde"), 19, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcde"), 19, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcdeabcdeabcde"), 19, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S(""), 20, 20);
    test(S("abcdeabcdeabcdeabcde"), S("abcde"), 20, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcde"), 20, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcdeabcdeabcde"), 20, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S(""), 21, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S("abcde"), 21, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcde"), 21, S::npos);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcdeabcdeabcde"), 21, S::npos);
}

template <class S>
void test1()
{
    test(S(""), S(""), 0);
    test(S(""), S("abcde"), S::npos);
    test(S(""), S("abcdeabcde"), S::npos);
    test(S(""), S("abcdeabcdeabcdeabcde"), S::npos);
    test(S("abcde"), S(""), 0);
    test(S("abcde"), S("abcde"), 0);
    test(S("abcde"), S("abcdeabcde"), S::npos);
    test(S("abcde"), S("abcdeabcdeabcdeabcde"), S::npos);
    test(S("abcdeabcde"), S(""), 0);
    test(S("abcdeabcde"), S("abcde"), 0);
    test(S("abcdeabcde"), S("abcdeabcde"), 0);
    test(S("abcdeabcde"), S("abcdeabcdeabcdeabcde"), S::npos);
    test(S("abcdeabcdeabcdeabcde"), S(""), 0);
    test(S("abcdeabcdeabcdeabcde"), S("abcde"), 0);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcde"), 0);
    test(S("abcdeabcdeabcdeabcde"), S("abcdeabcdeabcdeabcde"), 0);
}

int main()
{
    {
    typedef std::string S;
    test0<S>();
    test1<S>();
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test0<S>();
    test1<S>();
    }
#endif

#if TEST_STD_VER > 3
    {   // LWG 2946
    std::string s = " !";
    assert(s.find({"abc", 1}) == std::string::npos);
    }
#endif
}
