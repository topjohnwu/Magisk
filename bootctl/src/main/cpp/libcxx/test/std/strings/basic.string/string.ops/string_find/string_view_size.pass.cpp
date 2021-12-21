//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// size_type find(basic_string_view sv, size_type pos = 0) const;

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S, class SV>
void
test(const S& s, SV sv, typename S::size_type pos, typename S::size_type x)
{
    assert(s.find(sv, pos) == x);
    if (x != S::npos)
        assert(pos <= x && x + sv.size() <= s.size());
}

template <class S, class SV>
void
test(const S& s, SV sv, typename S::size_type x)
{
    assert(s.find(sv) == x);
    if (x != S::npos)
        assert(0 <= x && x + sv.size() <= s.size());
}

template <class S, class SV>
void test0()
{
    test(S(""), SV(""), 0, 0);
    test(S(""), SV("abcde"), 0, S::npos);
    test(S(""), SV("abcdeabcde"), 0, S::npos);
    test(S(""), SV("abcdeabcdeabcdeabcde"), 0, S::npos);
    test(S(""), SV(""), 1, S::npos);
    test(S(""), SV("abcde"), 1, S::npos);
    test(S(""), SV("abcdeabcde"), 1, S::npos);
    test(S(""), SV("abcdeabcdeabcdeabcde"), 1, S::npos);
    test(S("abcde"), SV(""), 0, 0);
    test(S("abcde"), SV("abcde"), 0, 0);
    test(S("abcde"), SV("abcdeabcde"), 0, S::npos);
    test(S("abcde"), SV("abcdeabcdeabcdeabcde"), 0, S::npos);
    test(S("abcde"), SV(""), 1, 1);
    test(S("abcde"), SV("abcde"), 1, S::npos);
    test(S("abcde"), SV("abcdeabcde"), 1, S::npos);
    test(S("abcde"), SV("abcdeabcdeabcdeabcde"), 1, S::npos);
    test(S("abcde"), SV(""), 2, 2);
    test(S("abcde"), SV("abcde"), 2, S::npos);
    test(S("abcde"), SV("abcdeabcde"), 2, S::npos);
    test(S("abcde"), SV("abcdeabcdeabcdeabcde"), 2, S::npos);
    test(S("abcde"), SV(""), 4, 4);
    test(S("abcde"), SV("abcde"), 4, S::npos);
    test(S("abcde"), SV("abcdeabcde"), 4, S::npos);
    test(S("abcde"), SV("abcdeabcdeabcdeabcde"), 4, S::npos);
    test(S("abcde"), SV(""), 5, 5);
    test(S("abcde"), SV("abcde"), 5, S::npos);
    test(S("abcde"), SV("abcdeabcde"), 5, S::npos);
    test(S("abcde"), SV("abcdeabcdeabcdeabcde"), 5, S::npos);
    test(S("abcde"), SV(""), 6, S::npos);
    test(S("abcde"), SV("abcde"), 6, S::npos);
    test(S("abcde"), SV("abcdeabcde"), 6, S::npos);
    test(S("abcde"), SV("abcdeabcdeabcdeabcde"), 6, S::npos);
    test(S("abcdeabcde"), SV(""), 0, 0);
    test(S("abcdeabcde"), SV("abcde"), 0, 0);
    test(S("abcdeabcde"), SV("abcdeabcde"), 0, 0);
    test(S("abcdeabcde"), SV("abcdeabcdeabcdeabcde"), 0, S::npos);
    test(S("abcdeabcde"), SV(""), 1, 1);
    test(S("abcdeabcde"), SV("abcde"), 1, 5);
    test(S("abcdeabcde"), SV("abcdeabcde"), 1, S::npos);
    test(S("abcdeabcde"), SV("abcdeabcdeabcdeabcde"), 1, S::npos);
    test(S("abcdeabcde"), SV(""), 5, 5);
    test(S("abcdeabcde"), SV("abcde"), 5, 5);
    test(S("abcdeabcde"), SV("abcdeabcde"), 5, S::npos);
    test(S("abcdeabcde"), SV("abcdeabcdeabcdeabcde"), 5, S::npos);
    test(S("abcdeabcde"), SV(""), 9, 9);
    test(S("abcdeabcde"), SV("abcde"), 9, S::npos);
    test(S("abcdeabcde"), SV("abcdeabcde"), 9, S::npos);
    test(S("abcdeabcde"), SV("abcdeabcdeabcdeabcde"), 9, S::npos);
    test(S("abcdeabcde"), SV(""), 10, 10);
    test(S("abcdeabcde"), SV("abcde"), 10, S::npos);
    test(S("abcdeabcde"), SV("abcdeabcde"), 10, S::npos);
    test(S("abcdeabcde"), SV("abcdeabcdeabcdeabcde"), 10, S::npos);
    test(S("abcdeabcde"), SV(""), 11, S::npos);
    test(S("abcdeabcde"), SV("abcde"), 11, S::npos);
    test(S("abcdeabcde"), SV("abcdeabcde"), 11, S::npos);
    test(S("abcdeabcde"), SV("abcdeabcdeabcdeabcde"), 11, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV(""), 0, 0);
    test(S("abcdeabcdeabcdeabcde"), SV("abcde"), 0, 0);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcde"), 0, 0);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcdeabcdeabcde"), 0, 0);
    test(S("abcdeabcdeabcdeabcde"), SV(""), 1, 1);
    test(S("abcdeabcdeabcdeabcde"), SV("abcde"), 1, 5);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcde"), 1, 5);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcdeabcdeabcde"), 1, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV(""), 10, 10);
    test(S("abcdeabcdeabcdeabcde"), SV("abcde"), 10, 10);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcde"), 10, 10);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcdeabcdeabcde"), 10, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV(""), 19, 19);
    test(S("abcdeabcdeabcdeabcde"), SV("abcde"), 19, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcde"), 19, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcdeabcdeabcde"), 19, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV(""), 20, 20);
    test(S("abcdeabcdeabcdeabcde"), SV("abcde"), 20, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcde"), 20, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcdeabcdeabcde"), 20, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV(""), 21, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV("abcde"), 21, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcde"), 21, S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcdeabcdeabcde"), 21, S::npos);
}

template <class S, class SV>
void test1()
{
    test(S(""), SV(""), 0);
    test(S(""), SV("abcde"), S::npos);
    test(S(""), SV("abcdeabcde"), S::npos);
    test(S(""), SV("abcdeabcdeabcdeabcde"), S::npos);
    test(S("abcde"), SV(""), 0);
    test(S("abcde"), SV("abcde"), 0);
    test(S("abcde"), SV("abcdeabcde"), S::npos);
    test(S("abcde"), SV("abcdeabcdeabcdeabcde"), S::npos);
    test(S("abcdeabcde"), SV(""), 0);
    test(S("abcdeabcde"), SV("abcde"), 0);
    test(S("abcdeabcde"), SV("abcdeabcde"), 0);
    test(S("abcdeabcde"), SV("abcdeabcdeabcdeabcde"), S::npos);
    test(S("abcdeabcdeabcdeabcde"), SV(""), 0);
    test(S("abcdeabcdeabcdeabcde"), SV("abcde"), 0);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcde"), 0);
    test(S("abcdeabcdeabcdeabcde"), SV("abcdeabcdeabcdeabcde"), 0);
}

int main()
{
    {
    typedef std::string S;
    typedef std::string_view SV;
    test0<S, SV>();
    test1<S, SV>();
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    typedef std::string_view SV;
    test0<S, SV>();
    test1<S, SV>();
    }
#endif
}
