//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type rfind(charT c, size_type pos = npos) const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type pos,
     typename S::size_type x)
{
    assert(s.rfind(c, pos) == x);
    if (x != S::npos)
        assert(x <= pos && x + 1 <= s.size());
}

template <class S>
void
test(const S& s, typename S::value_type c, typename S::size_type x)
{
    assert(s.rfind(c) == x);
    if (x != S::npos)
        assert(x + 1 <= s.size());
}

int main()
{
    {
    typedef std::string S;
    test(S(""), 'b', 0, S::npos);
    test(S(""), 'b', 1, S::npos);
    test(S("abcde"), 'b', 0, S::npos);
    test(S("abcde"), 'b', 1, 1);
    test(S("abcde"), 'b', 2, 1);
    test(S("abcde"), 'b', 4, 1);
    test(S("abcde"), 'b', 5, 1);
    test(S("abcde"), 'b', 6, 1);
    test(S("abcdeabcde"), 'b', 0, S::npos);
    test(S("abcdeabcde"), 'b', 1, 1);
    test(S("abcdeabcde"), 'b', 5, 1);
    test(S("abcdeabcde"), 'b', 9, 6);
    test(S("abcdeabcde"), 'b', 10, 6);
    test(S("abcdeabcde"), 'b', 11, 6);
    test(S("abcdeabcdeabcdeabcde"), 'b', 0, S::npos);
    test(S("abcdeabcdeabcdeabcde"), 'b', 1, 1);
    test(S("abcdeabcdeabcdeabcde"), 'b', 10, 6);
    test(S("abcdeabcdeabcdeabcde"), 'b', 19, 16);
    test(S("abcdeabcdeabcdeabcde"), 'b', 20, 16);
    test(S("abcdeabcdeabcdeabcde"), 'b', 21, 16);

    test(S(""), 'b', S::npos);
    test(S("abcde"), 'b', 1);
    test(S("abcdeabcde"), 'b', 6);
    test(S("abcdeabcdeabcdeabcde"), 'b', 16);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), 'b', 0, S::npos);
    test(S(""), 'b', 1, S::npos);
    test(S("abcde"), 'b', 0, S::npos);
    test(S("abcde"), 'b', 1, 1);
    test(S("abcde"), 'b', 2, 1);
    test(S("abcde"), 'b', 4, 1);
    test(S("abcde"), 'b', 5, 1);
    test(S("abcde"), 'b', 6, 1);
    test(S("abcdeabcde"), 'b', 0, S::npos);
    test(S("abcdeabcde"), 'b', 1, 1);
    test(S("abcdeabcde"), 'b', 5, 1);
    test(S("abcdeabcde"), 'b', 9, 6);
    test(S("abcdeabcde"), 'b', 10, 6);
    test(S("abcdeabcde"), 'b', 11, 6);
    test(S("abcdeabcdeabcdeabcde"), 'b', 0, S::npos);
    test(S("abcdeabcdeabcdeabcde"), 'b', 1, 1);
    test(S("abcdeabcdeabcdeabcde"), 'b', 10, 6);
    test(S("abcdeabcdeabcdeabcde"), 'b', 19, 16);
    test(S("abcdeabcdeabcdeabcde"), 'b', 20, 16);
    test(S("abcdeabcdeabcdeabcde"), 'b', 21, 16);

    test(S(""), 'b', S::npos);
    test(S("abcde"), 'b', 1);
    test(S("abcdeabcde"), 'b', 6);
    test(S("abcdeabcdeabcdeabcde"), 'b', 16);
    }
#endif
}
