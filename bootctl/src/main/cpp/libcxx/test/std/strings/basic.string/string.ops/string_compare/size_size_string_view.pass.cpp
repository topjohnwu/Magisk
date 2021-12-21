//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// int compare(size_type pos1, size_type n1, basic_string_vew sv) const;

#include <string>
#include <stdexcept>
#include <cassert>

#include "min_allocator.h"

#include "test_macros.h"

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
test(const S& s, typename S::size_type pos1, typename S::size_type n1,
     SV sv, int x)
{
    if (pos1 <= s.size())
        assert(sign(s.compare(pos1, n1, sv)) == sign(x));
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            TEST_IGNORE_NODISCARD s.compare(pos1, n1, sv);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos1 > s.size());
        }
    }
#endif
}

template <class S, class SV>
void test0()
{
    test(S(""), 0, 0, SV(""), 0);
    test(S(""), 0, 0, SV("abcde"), -5);
    test(S(""), 0, 0, SV("abcdefghij"), -10);
    test(S(""), 0, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S(""), 0, 1, SV(""), 0);
    test(S(""), 0, 1, SV("abcde"), -5);
    test(S(""), 0, 1, SV("abcdefghij"), -10);
    test(S(""), 0, 1, SV("abcdefghijklmnopqrst"), -20);
    test(S(""), 1, 0, SV(""), 0);
    test(S(""), 1, 0, SV("abcde"), 0);
    test(S(""), 1, 0, SV("abcdefghij"), 0);
    test(S(""), 1, 0, SV("abcdefghijklmnopqrst"), 0);
    test(S("abcde"), 0, 0, SV(""), 0);
    test(S("abcde"), 0, 0, SV("abcde"), -5);
    test(S("abcde"), 0, 0, SV("abcdefghij"), -10);
    test(S("abcde"), 0, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 0, 1, SV(""), 1);
    test(S("abcde"), 0, 1, SV("abcde"), -4);
    test(S("abcde"), 0, 1, SV("abcdefghij"), -9);
    test(S("abcde"), 0, 1, SV("abcdefghijklmnopqrst"), -19);
    test(S("abcde"), 0, 2, SV(""), 2);
    test(S("abcde"), 0, 2, SV("abcde"), -3);
    test(S("abcde"), 0, 2, SV("abcdefghij"), -8);
    test(S("abcde"), 0, 2, SV("abcdefghijklmnopqrst"), -18);
    test(S("abcde"), 0, 4, SV(""), 4);
    test(S("abcde"), 0, 4, SV("abcde"), -1);
    test(S("abcde"), 0, 4, SV("abcdefghij"), -6);
    test(S("abcde"), 0, 4, SV("abcdefghijklmnopqrst"), -16);
    test(S("abcde"), 0, 5, SV(""), 5);
    test(S("abcde"), 0, 5, SV("abcde"), 0);
    test(S("abcde"), 0, 5, SV("abcdefghij"), -5);
    test(S("abcde"), 0, 5, SV("abcdefghijklmnopqrst"), -15);
    test(S("abcde"), 0, 6, SV(""), 5);
    test(S("abcde"), 0, 6, SV("abcde"), 0);
    test(S("abcde"), 0, 6, SV("abcdefghij"), -5);
    test(S("abcde"), 0, 6, SV("abcdefghijklmnopqrst"), -15);
    test(S("abcde"), 1, 0, SV(""), 0);
    test(S("abcde"), 1, 0, SV("abcde"), -5);
    test(S("abcde"), 1, 0, SV("abcdefghij"), -10);
    test(S("abcde"), 1, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 1, 1, SV(""), 1);
    test(S("abcde"), 1, 1, SV("abcde"), 1);
    test(S("abcde"), 1, 1, SV("abcdefghij"), 1);
    test(S("abcde"), 1, 1, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 1, 2, SV(""), 2);
    test(S("abcde"), 1, 2, SV("abcde"), 1);
    test(S("abcde"), 1, 2, SV("abcdefghij"), 1);
    test(S("abcde"), 1, 2, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 1, 3, SV(""), 3);
    test(S("abcde"), 1, 3, SV("abcde"), 1);
    test(S("abcde"), 1, 3, SV("abcdefghij"), 1);
    test(S("abcde"), 1, 3, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 1, 4, SV(""), 4);
    test(S("abcde"), 1, 4, SV("abcde"), 1);
    test(S("abcde"), 1, 4, SV("abcdefghij"), 1);
    test(S("abcde"), 1, 4, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 1, 5, SV(""), 4);
    test(S("abcde"), 1, 5, SV("abcde"), 1);
    test(S("abcde"), 1, 5, SV("abcdefghij"), 1);
    test(S("abcde"), 1, 5, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 2, 0, SV(""), 0);
    test(S("abcde"), 2, 0, SV("abcde"), -5);
    test(S("abcde"), 2, 0, SV("abcdefghij"), -10);
    test(S("abcde"), 2, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 2, 1, SV(""), 1);
    test(S("abcde"), 2, 1, SV("abcde"), 2);
    test(S("abcde"), 2, 1, SV("abcdefghij"), 2);
    test(S("abcde"), 2, 1, SV("abcdefghijklmnopqrst"), 2);
    test(S("abcde"), 2, 2, SV(""), 2);
    test(S("abcde"), 2, 2, SV("abcde"), 2);
    test(S("abcde"), 2, 2, SV("abcdefghij"), 2);
    test(S("abcde"), 2, 2, SV("abcdefghijklmnopqrst"), 2);
    test(S("abcde"), 2, 3, SV(""), 3);
    test(S("abcde"), 2, 3, SV("abcde"), 2);
    test(S("abcde"), 2, 3, SV("abcdefghij"), 2);
    test(S("abcde"), 2, 3, SV("abcdefghijklmnopqrst"), 2);
    test(S("abcde"), 2, 4, SV(""), 3);
    test(S("abcde"), 2, 4, SV("abcde"), 2);
    test(S("abcde"), 2, 4, SV("abcdefghij"), 2);
    test(S("abcde"), 2, 4, SV("abcdefghijklmnopqrst"), 2);
    test(S("abcde"), 4, 0, SV(""), 0);
    test(S("abcde"), 4, 0, SV("abcde"), -5);
    test(S("abcde"), 4, 0, SV("abcdefghij"), -10);
    test(S("abcde"), 4, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 4, 1, SV(""), 1);
    test(S("abcde"), 4, 1, SV("abcde"), 4);
    test(S("abcde"), 4, 1, SV("abcdefghij"), 4);
    test(S("abcde"), 4, 1, SV("abcdefghijklmnopqrst"), 4);
    test(S("abcde"), 4, 2, SV(""), 1);
    test(S("abcde"), 4, 2, SV("abcde"), 4);
    test(S("abcde"), 4, 2, SV("abcdefghij"), 4);
    test(S("abcde"), 4, 2, SV("abcdefghijklmnopqrst"), 4);
    test(S("abcde"), 5, 0, SV(""), 0);
    test(S("abcde"), 5, 0, SV("abcde"), -5);
    test(S("abcde"), 5, 0, SV("abcdefghij"), -10);
    test(S("abcde"), 5, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 5, 1, SV(""), 0);
    test(S("abcde"), 5, 1, SV("abcde"), -5);
    test(S("abcde"), 5, 1, SV("abcdefghij"), -10);
    test(S("abcde"), 5, 1, SV("abcdefghijklmnopqrst"), -20);
}

template <class S, class SV>
void test1()
{
    test(S("abcde"), 6, 0, SV(""), 0);
    test(S("abcde"), 6, 0, SV("abcde"), 0);
    test(S("abcde"), 6, 0, SV("abcdefghij"), 0);
    test(S("abcde"), 6, 0, SV("abcdefghijklmnopqrst"), 0);
    test(S("abcdefghij"), 0, 0, SV(""), 0);
    test(S("abcdefghij"), 0, 0, SV("abcde"), -5);
    test(S("abcdefghij"), 0, 0, SV("abcdefghij"), -10);
    test(S("abcdefghij"), 0, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 0, 1, SV(""), 1);
    test(S("abcdefghij"), 0, 1, SV("abcde"), -4);
    test(S("abcdefghij"), 0, 1, SV("abcdefghij"), -9);
    test(S("abcdefghij"), 0, 1, SV("abcdefghijklmnopqrst"), -19);
    test(S("abcdefghij"), 0, 5, SV(""), 5);
    test(S("abcdefghij"), 0, 5, SV("abcde"), 0);
    test(S("abcdefghij"), 0, 5, SV("abcdefghij"), -5);
    test(S("abcdefghij"), 0, 5, SV("abcdefghijklmnopqrst"), -15);
    test(S("abcdefghij"), 0, 9, SV(""), 9);
    test(S("abcdefghij"), 0, 9, SV("abcde"), 4);
    test(S("abcdefghij"), 0, 9, SV("abcdefghij"), -1);
    test(S("abcdefghij"), 0, 9, SV("abcdefghijklmnopqrst"), -11);
    test(S("abcdefghij"), 0, 10, SV(""), 10);
    test(S("abcdefghij"), 0, 10, SV("abcde"), 5);
    test(S("abcdefghij"), 0, 10, SV("abcdefghij"), 0);
    test(S("abcdefghij"), 0, 10, SV("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghij"), 0, 11, SV(""), 10);
    test(S("abcdefghij"), 0, 11, SV("abcde"), 5);
    test(S("abcdefghij"), 0, 11, SV("abcdefghij"), 0);
    test(S("abcdefghij"), 0, 11, SV("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghij"), 1, 0, SV(""), 0);
    test(S("abcdefghij"), 1, 0, SV("abcde"), -5);
    test(S("abcdefghij"), 1, 0, SV("abcdefghij"), -10);
    test(S("abcdefghij"), 1, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 1, 1, SV(""), 1);
    test(S("abcdefghij"), 1, 1, SV("abcde"), 1);
    test(S("abcdefghij"), 1, 1, SV("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 1, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 1, 4, SV(""), 4);
    test(S("abcdefghij"), 1, 4, SV("abcde"), 1);
    test(S("abcdefghij"), 1, 4, SV("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 4, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 1, 8, SV(""), 8);
    test(S("abcdefghij"), 1, 8, SV("abcde"), 1);
    test(S("abcdefghij"), 1, 8, SV("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 8, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 1, 9, SV(""), 9);
    test(S("abcdefghij"), 1, 9, SV("abcde"), 1);
    test(S("abcdefghij"), 1, 9, SV("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 9, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 1, 10, SV(""), 9);
    test(S("abcdefghij"), 1, 10, SV("abcde"), 1);
    test(S("abcdefghij"), 1, 10, SV("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 10, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 5, 0, SV(""), 0);
    test(S("abcdefghij"), 5, 0, SV("abcde"), -5);
    test(S("abcdefghij"), 5, 0, SV("abcdefghij"), -10);
    test(S("abcdefghij"), 5, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 5, 1, SV(""), 1);
    test(S("abcdefghij"), 5, 1, SV("abcde"), 5);
    test(S("abcdefghij"), 5, 1, SV("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 1, SV("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 5, 2, SV(""), 2);
    test(S("abcdefghij"), 5, 2, SV("abcde"), 5);
    test(S("abcdefghij"), 5, 2, SV("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 2, SV("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 5, 4, SV(""), 4);
    test(S("abcdefghij"), 5, 4, SV("abcde"), 5);
    test(S("abcdefghij"), 5, 4, SV("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 4, SV("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 5, 5, SV(""), 5);
    test(S("abcdefghij"), 5, 5, SV("abcde"), 5);
    test(S("abcdefghij"), 5, 5, SV("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 5, SV("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 5, 6, SV(""), 5);
    test(S("abcdefghij"), 5, 6, SV("abcde"), 5);
    test(S("abcdefghij"), 5, 6, SV("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 6, SV("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 9, 0, SV(""), 0);
    test(S("abcdefghij"), 9, 0, SV("abcde"), -5);
    test(S("abcdefghij"), 9, 0, SV("abcdefghij"), -10);
    test(S("abcdefghij"), 9, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 9, 1, SV(""), 1);
    test(S("abcdefghij"), 9, 1, SV("abcde"), 9);
    test(S("abcdefghij"), 9, 1, SV("abcdefghij"), 9);
    test(S("abcdefghij"), 9, 1, SV("abcdefghijklmnopqrst"), 9);
    test(S("abcdefghij"), 9, 2, SV(""), 1);
    test(S("abcdefghij"), 9, 2, SV("abcde"), 9);
    test(S("abcdefghij"), 9, 2, SV("abcdefghij"), 9);
    test(S("abcdefghij"), 9, 2, SV("abcdefghijklmnopqrst"), 9);
    test(S("abcdefghij"), 10, 0, SV(""), 0);
    test(S("abcdefghij"), 10, 0, SV("abcde"), -5);
    test(S("abcdefghij"), 10, 0, SV("abcdefghij"), -10);
    test(S("abcdefghij"), 10, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 10, 1, SV(""), 0);
    test(S("abcdefghij"), 10, 1, SV("abcde"), -5);
    test(S("abcdefghij"), 10, 1, SV("abcdefghij"), -10);
    test(S("abcdefghij"), 10, 1, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 11, 0, SV(""), 0);
    test(S("abcdefghij"), 11, 0, SV("abcde"), 0);
    test(S("abcdefghij"), 11, 0, SV("abcdefghij"), 0);
    test(S("abcdefghij"), 11, 0, SV("abcdefghijklmnopqrst"), 0);
}

template <class S, class SV>
void test2()
{
    test(S("abcdefghijklmnopqrst"), 0, 0, SV(""), 0);
    test(S("abcdefghijklmnopqrst"), 0, 0, SV("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 0, 0, SV("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 0, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 0, 1, SV(""), 1);
    test(S("abcdefghijklmnopqrst"), 0, 1, SV("abcde"), -4);
    test(S("abcdefghijklmnopqrst"), 0, 1, SV("abcdefghij"), -9);
    test(S("abcdefghijklmnopqrst"), 0, 1, SV("abcdefghijklmnopqrst"), -19);
    test(S("abcdefghijklmnopqrst"), 0, 10, SV(""), 10);
    test(S("abcdefghijklmnopqrst"), 0, 10, SV("abcde"), 5);
    test(S("abcdefghijklmnopqrst"), 0, 10, SV("abcdefghij"), 0);
    test(S("abcdefghijklmnopqrst"), 0, 10, SV("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghijklmnopqrst"), 0, 19, SV(""), 19);
    test(S("abcdefghijklmnopqrst"), 0, 19, SV("abcde"), 14);
    test(S("abcdefghijklmnopqrst"), 0, 19, SV("abcdefghij"), 9);
    test(S("abcdefghijklmnopqrst"), 0, 19, SV("abcdefghijklmnopqrst"), -1);
    test(S("abcdefghijklmnopqrst"), 0, 20, SV(""), 20);
    test(S("abcdefghijklmnopqrst"), 0, 20, SV("abcde"), 15);
    test(S("abcdefghijklmnopqrst"), 0, 20, SV("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 0, 20, SV("abcdefghijklmnopqrst"), 0);
    test(S("abcdefghijklmnopqrst"), 0, 21, SV(""), 20);
    test(S("abcdefghijklmnopqrst"), 0, 21, SV("abcde"), 15);
    test(S("abcdefghijklmnopqrst"), 0, 21, SV("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 0, 21, SV("abcdefghijklmnopqrst"), 0);
    test(S("abcdefghijklmnopqrst"), 1, 0, SV(""), 0);
    test(S("abcdefghijklmnopqrst"), 1, 0, SV("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 1, 0, SV("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 1, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 1, 1, SV(""), 1);
    test(S("abcdefghijklmnopqrst"), 1, 1, SV("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 1, SV("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 1, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 9, SV(""), 9);
    test(S("abcdefghijklmnopqrst"), 1, 9, SV("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 9, SV("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 9, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 18, SV(""), 18);
    test(S("abcdefghijklmnopqrst"), 1, 18, SV("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 18, SV("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 18, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 19, SV(""), 19);
    test(S("abcdefghijklmnopqrst"), 1, 19, SV("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 19, SV("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 19, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 20, SV(""), 19);
    test(S("abcdefghijklmnopqrst"), 1, 20, SV("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 20, SV("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 20, SV("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 10, 0, SV(""), 0);
    test(S("abcdefghijklmnopqrst"), 10, 0, SV("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 10, 0, SV("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 10, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 10, 1, SV(""), 1);
    test(S("abcdefghijklmnopqrst"), 10, 1, SV("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 1, SV("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 1, SV("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 5, SV(""), 5);
    test(S("abcdefghijklmnopqrst"), 10, 5, SV("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 5, SV("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 5, SV("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 9, SV(""), 9);
    test(S("abcdefghijklmnopqrst"), 10, 9, SV("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 9, SV("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 9, SV("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, SV(""), 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, SV("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, SV("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, SV("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, SV(""), 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, SV("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, SV("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, SV("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 19, 0, SV(""), 0);
    test(S("abcdefghijklmnopqrst"), 19, 0, SV("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 19, 0, SV("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 19, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 19, 1, SV(""), 1);
    test(S("abcdefghijklmnopqrst"), 19, 1, SV("abcde"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 1, SV("abcdefghij"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 1, SV("abcdefghijklmnopqrst"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 2, SV(""), 1);
    test(S("abcdefghijklmnopqrst"), 19, 2, SV("abcde"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 2, SV("abcdefghij"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 2, SV("abcdefghijklmnopqrst"), 19);
    test(S("abcdefghijklmnopqrst"), 20, 0, SV(""), 0);
    test(S("abcdefghijklmnopqrst"), 20, 0, SV("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 20, 0, SV("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 20, 0, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 20, 1, SV(""), 0);
    test(S("abcdefghijklmnopqrst"), 20, 1, SV("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 20, 1, SV("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 20, 1, SV("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 21, 0, SV(""), 0);
    test(S("abcdefghijklmnopqrst"), 21, 0, SV("abcde"), 0);
    test(S("abcdefghijklmnopqrst"), 21, 0, SV("abcdefghij"), 0);
    test(S("abcdefghijklmnopqrst"), 21, 0, SV("abcdefghijklmnopqrst"), 0);
}

int main()
{
    {
    typedef std::string S;
    typedef std::string_view SV;
    test0<S, SV>();
    test1<S, SV>();
    test2<S, SV>();
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    typedef std::string_view SV;
    test0<S, SV>();
    test1<S, SV>();
    test2<S, SV>();
    }
#endif
}
