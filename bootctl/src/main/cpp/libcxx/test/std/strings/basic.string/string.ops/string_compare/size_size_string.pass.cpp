//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// int compare(size_type pos1, size_type n1, const basic_string& str) const;

#include <string>
#include <stdexcept>
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
test(const S& s, typename S::size_type pos1, typename S::size_type n1,
     const S& str, int x)
{
    if (pos1 <= s.size())
        assert(sign(s.compare(pos1, n1, str)) == sign(x));
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            TEST_IGNORE_NODISCARD s.compare(pos1, n1, str);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos1 > s.size());
        }
    }
#endif
}

template <class S>
void test0()
{
    test(S(""), 0, 0, S(""), 0);
    test(S(""), 0, 0, S("abcde"), -5);
    test(S(""), 0, 0, S("abcdefghij"), -10);
    test(S(""), 0, 0, S("abcdefghijklmnopqrst"), -20);
    test(S(""), 0, 1, S(""), 0);
    test(S(""), 0, 1, S("abcde"), -5);
    test(S(""), 0, 1, S("abcdefghij"), -10);
    test(S(""), 0, 1, S("abcdefghijklmnopqrst"), -20);
    test(S(""), 1, 0, S(""), 0);
    test(S(""), 1, 0, S("abcde"), 0);
    test(S(""), 1, 0, S("abcdefghij"), 0);
    test(S(""), 1, 0, S("abcdefghijklmnopqrst"), 0);
    test(S("abcde"), 0, 0, S(""), 0);
    test(S("abcde"), 0, 0, S("abcde"), -5);
    test(S("abcde"), 0, 0, S("abcdefghij"), -10);
    test(S("abcde"), 0, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 0, 1, S(""), 1);
    test(S("abcde"), 0, 1, S("abcde"), -4);
    test(S("abcde"), 0, 1, S("abcdefghij"), -9);
    test(S("abcde"), 0, 1, S("abcdefghijklmnopqrst"), -19);
    test(S("abcde"), 0, 2, S(""), 2);
    test(S("abcde"), 0, 2, S("abcde"), -3);
    test(S("abcde"), 0, 2, S("abcdefghij"), -8);
    test(S("abcde"), 0, 2, S("abcdefghijklmnopqrst"), -18);
    test(S("abcde"), 0, 4, S(""), 4);
    test(S("abcde"), 0, 4, S("abcde"), -1);
    test(S("abcde"), 0, 4, S("abcdefghij"), -6);
    test(S("abcde"), 0, 4, S("abcdefghijklmnopqrst"), -16);
    test(S("abcde"), 0, 5, S(""), 5);
    test(S("abcde"), 0, 5, S("abcde"), 0);
    test(S("abcde"), 0, 5, S("abcdefghij"), -5);
    test(S("abcde"), 0, 5, S("abcdefghijklmnopqrst"), -15);
    test(S("abcde"), 0, 6, S(""), 5);
    test(S("abcde"), 0, 6, S("abcde"), 0);
    test(S("abcde"), 0, 6, S("abcdefghij"), -5);
    test(S("abcde"), 0, 6, S("abcdefghijklmnopqrst"), -15);
    test(S("abcde"), 1, 0, S(""), 0);
    test(S("abcde"), 1, 0, S("abcde"), -5);
    test(S("abcde"), 1, 0, S("abcdefghij"), -10);
    test(S("abcde"), 1, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 1, 1, S(""), 1);
    test(S("abcde"), 1, 1, S("abcde"), 1);
    test(S("abcde"), 1, 1, S("abcdefghij"), 1);
    test(S("abcde"), 1, 1, S("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 1, 2, S(""), 2);
    test(S("abcde"), 1, 2, S("abcde"), 1);
    test(S("abcde"), 1, 2, S("abcdefghij"), 1);
    test(S("abcde"), 1, 2, S("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 1, 3, S(""), 3);
    test(S("abcde"), 1, 3, S("abcde"), 1);
    test(S("abcde"), 1, 3, S("abcdefghij"), 1);
    test(S("abcde"), 1, 3, S("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 1, 4, S(""), 4);
    test(S("abcde"), 1, 4, S("abcde"), 1);
    test(S("abcde"), 1, 4, S("abcdefghij"), 1);
    test(S("abcde"), 1, 4, S("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 1, 5, S(""), 4);
    test(S("abcde"), 1, 5, S("abcde"), 1);
    test(S("abcde"), 1, 5, S("abcdefghij"), 1);
    test(S("abcde"), 1, 5, S("abcdefghijklmnopqrst"), 1);
    test(S("abcde"), 2, 0, S(""), 0);
    test(S("abcde"), 2, 0, S("abcde"), -5);
    test(S("abcde"), 2, 0, S("abcdefghij"), -10);
    test(S("abcde"), 2, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 2, 1, S(""), 1);
    test(S("abcde"), 2, 1, S("abcde"), 2);
    test(S("abcde"), 2, 1, S("abcdefghij"), 2);
    test(S("abcde"), 2, 1, S("abcdefghijklmnopqrst"), 2);
    test(S("abcde"), 2, 2, S(""), 2);
    test(S("abcde"), 2, 2, S("abcde"), 2);
    test(S("abcde"), 2, 2, S("abcdefghij"), 2);
    test(S("abcde"), 2, 2, S("abcdefghijklmnopqrst"), 2);
    test(S("abcde"), 2, 3, S(""), 3);
    test(S("abcde"), 2, 3, S("abcde"), 2);
    test(S("abcde"), 2, 3, S("abcdefghij"), 2);
    test(S("abcde"), 2, 3, S("abcdefghijklmnopqrst"), 2);
    test(S("abcde"), 2, 4, S(""), 3);
    test(S("abcde"), 2, 4, S("abcde"), 2);
    test(S("abcde"), 2, 4, S("abcdefghij"), 2);
    test(S("abcde"), 2, 4, S("abcdefghijklmnopqrst"), 2);
    test(S("abcde"), 4, 0, S(""), 0);
    test(S("abcde"), 4, 0, S("abcde"), -5);
    test(S("abcde"), 4, 0, S("abcdefghij"), -10);
    test(S("abcde"), 4, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 4, 1, S(""), 1);
    test(S("abcde"), 4, 1, S("abcde"), 4);
    test(S("abcde"), 4, 1, S("abcdefghij"), 4);
    test(S("abcde"), 4, 1, S("abcdefghijklmnopqrst"), 4);
    test(S("abcde"), 4, 2, S(""), 1);
    test(S("abcde"), 4, 2, S("abcde"), 4);
    test(S("abcde"), 4, 2, S("abcdefghij"), 4);
    test(S("abcde"), 4, 2, S("abcdefghijklmnopqrst"), 4);
    test(S("abcde"), 5, 0, S(""), 0);
    test(S("abcde"), 5, 0, S("abcde"), -5);
    test(S("abcde"), 5, 0, S("abcdefghij"), -10);
    test(S("abcde"), 5, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcde"), 5, 1, S(""), 0);
    test(S("abcde"), 5, 1, S("abcde"), -5);
    test(S("abcde"), 5, 1, S("abcdefghij"), -10);
    test(S("abcde"), 5, 1, S("abcdefghijklmnopqrst"), -20);
}

template <class S>
void test1()
{
    test(S("abcde"), 6, 0, S(""), 0);
    test(S("abcde"), 6, 0, S("abcde"), 0);
    test(S("abcde"), 6, 0, S("abcdefghij"), 0);
    test(S("abcde"), 6, 0, S("abcdefghijklmnopqrst"), 0);
    test(S("abcdefghij"), 0, 0, S(""), 0);
    test(S("abcdefghij"), 0, 0, S("abcde"), -5);
    test(S("abcdefghij"), 0, 0, S("abcdefghij"), -10);
    test(S("abcdefghij"), 0, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 0, 1, S(""), 1);
    test(S("abcdefghij"), 0, 1, S("abcde"), -4);
    test(S("abcdefghij"), 0, 1, S("abcdefghij"), -9);
    test(S("abcdefghij"), 0, 1, S("abcdefghijklmnopqrst"), -19);
    test(S("abcdefghij"), 0, 5, S(""), 5);
    test(S("abcdefghij"), 0, 5, S("abcde"), 0);
    test(S("abcdefghij"), 0, 5, S("abcdefghij"), -5);
    test(S("abcdefghij"), 0, 5, S("abcdefghijklmnopqrst"), -15);
    test(S("abcdefghij"), 0, 9, S(""), 9);
    test(S("abcdefghij"), 0, 9, S("abcde"), 4);
    test(S("abcdefghij"), 0, 9, S("abcdefghij"), -1);
    test(S("abcdefghij"), 0, 9, S("abcdefghijklmnopqrst"), -11);
    test(S("abcdefghij"), 0, 10, S(""), 10);
    test(S("abcdefghij"), 0, 10, S("abcde"), 5);
    test(S("abcdefghij"), 0, 10, S("abcdefghij"), 0);
    test(S("abcdefghij"), 0, 10, S("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghij"), 0, 11, S(""), 10);
    test(S("abcdefghij"), 0, 11, S("abcde"), 5);
    test(S("abcdefghij"), 0, 11, S("abcdefghij"), 0);
    test(S("abcdefghij"), 0, 11, S("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghij"), 1, 0, S(""), 0);
    test(S("abcdefghij"), 1, 0, S("abcde"), -5);
    test(S("abcdefghij"), 1, 0, S("abcdefghij"), -10);
    test(S("abcdefghij"), 1, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 1, 1, S(""), 1);
    test(S("abcdefghij"), 1, 1, S("abcde"), 1);
    test(S("abcdefghij"), 1, 1, S("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 1, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 1, 4, S(""), 4);
    test(S("abcdefghij"), 1, 4, S("abcde"), 1);
    test(S("abcdefghij"), 1, 4, S("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 4, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 1, 8, S(""), 8);
    test(S("abcdefghij"), 1, 8, S("abcde"), 1);
    test(S("abcdefghij"), 1, 8, S("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 8, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 1, 9, S(""), 9);
    test(S("abcdefghij"), 1, 9, S("abcde"), 1);
    test(S("abcdefghij"), 1, 9, S("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 9, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 1, 10, S(""), 9);
    test(S("abcdefghij"), 1, 10, S("abcde"), 1);
    test(S("abcdefghij"), 1, 10, S("abcdefghij"), 1);
    test(S("abcdefghij"), 1, 10, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghij"), 5, 0, S(""), 0);
    test(S("abcdefghij"), 5, 0, S("abcde"), -5);
    test(S("abcdefghij"), 5, 0, S("abcdefghij"), -10);
    test(S("abcdefghij"), 5, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 5, 1, S(""), 1);
    test(S("abcdefghij"), 5, 1, S("abcde"), 5);
    test(S("abcdefghij"), 5, 1, S("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 1, S("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 5, 2, S(""), 2);
    test(S("abcdefghij"), 5, 2, S("abcde"), 5);
    test(S("abcdefghij"), 5, 2, S("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 2, S("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 5, 4, S(""), 4);
    test(S("abcdefghij"), 5, 4, S("abcde"), 5);
    test(S("abcdefghij"), 5, 4, S("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 4, S("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 5, 5, S(""), 5);
    test(S("abcdefghij"), 5, 5, S("abcde"), 5);
    test(S("abcdefghij"), 5, 5, S("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 5, S("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 5, 6, S(""), 5);
    test(S("abcdefghij"), 5, 6, S("abcde"), 5);
    test(S("abcdefghij"), 5, 6, S("abcdefghij"), 5);
    test(S("abcdefghij"), 5, 6, S("abcdefghijklmnopqrst"), 5);
    test(S("abcdefghij"), 9, 0, S(""), 0);
    test(S("abcdefghij"), 9, 0, S("abcde"), -5);
    test(S("abcdefghij"), 9, 0, S("abcdefghij"), -10);
    test(S("abcdefghij"), 9, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 9, 1, S(""), 1);
    test(S("abcdefghij"), 9, 1, S("abcde"), 9);
    test(S("abcdefghij"), 9, 1, S("abcdefghij"), 9);
    test(S("abcdefghij"), 9, 1, S("abcdefghijklmnopqrst"), 9);
    test(S("abcdefghij"), 9, 2, S(""), 1);
    test(S("abcdefghij"), 9, 2, S("abcde"), 9);
    test(S("abcdefghij"), 9, 2, S("abcdefghij"), 9);
    test(S("abcdefghij"), 9, 2, S("abcdefghijklmnopqrst"), 9);
    test(S("abcdefghij"), 10, 0, S(""), 0);
    test(S("abcdefghij"), 10, 0, S("abcde"), -5);
    test(S("abcdefghij"), 10, 0, S("abcdefghij"), -10);
    test(S("abcdefghij"), 10, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 10, 1, S(""), 0);
    test(S("abcdefghij"), 10, 1, S("abcde"), -5);
    test(S("abcdefghij"), 10, 1, S("abcdefghij"), -10);
    test(S("abcdefghij"), 10, 1, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghij"), 11, 0, S(""), 0);
    test(S("abcdefghij"), 11, 0, S("abcde"), 0);
    test(S("abcdefghij"), 11, 0, S("abcdefghij"), 0);
    test(S("abcdefghij"), 11, 0, S("abcdefghijklmnopqrst"), 0);
}

template <class S>
void test2()
{
    test(S("abcdefghijklmnopqrst"), 0, 0, S(""), 0);
    test(S("abcdefghijklmnopqrst"), 0, 0, S("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 0, 0, S("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 0, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 0, 1, S(""), 1);
    test(S("abcdefghijklmnopqrst"), 0, 1, S("abcde"), -4);
    test(S("abcdefghijklmnopqrst"), 0, 1, S("abcdefghij"), -9);
    test(S("abcdefghijklmnopqrst"), 0, 1, S("abcdefghijklmnopqrst"), -19);
    test(S("abcdefghijklmnopqrst"), 0, 10, S(""), 10);
    test(S("abcdefghijklmnopqrst"), 0, 10, S("abcde"), 5);
    test(S("abcdefghijklmnopqrst"), 0, 10, S("abcdefghij"), 0);
    test(S("abcdefghijklmnopqrst"), 0, 10, S("abcdefghijklmnopqrst"), -10);
    test(S("abcdefghijklmnopqrst"), 0, 19, S(""), 19);
    test(S("abcdefghijklmnopqrst"), 0, 19, S("abcde"), 14);
    test(S("abcdefghijklmnopqrst"), 0, 19, S("abcdefghij"), 9);
    test(S("abcdefghijklmnopqrst"), 0, 19, S("abcdefghijklmnopqrst"), -1);
    test(S("abcdefghijklmnopqrst"), 0, 20, S(""), 20);
    test(S("abcdefghijklmnopqrst"), 0, 20, S("abcde"), 15);
    test(S("abcdefghijklmnopqrst"), 0, 20, S("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 0, 20, S("abcdefghijklmnopqrst"), 0);
    test(S("abcdefghijklmnopqrst"), 0, 21, S(""), 20);
    test(S("abcdefghijklmnopqrst"), 0, 21, S("abcde"), 15);
    test(S("abcdefghijklmnopqrst"), 0, 21, S("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 0, 21, S("abcdefghijklmnopqrst"), 0);
    test(S("abcdefghijklmnopqrst"), 1, 0, S(""), 0);
    test(S("abcdefghijklmnopqrst"), 1, 0, S("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 1, 0, S("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 1, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 1, 1, S(""), 1);
    test(S("abcdefghijklmnopqrst"), 1, 1, S("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 1, S("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 1, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 9, S(""), 9);
    test(S("abcdefghijklmnopqrst"), 1, 9, S("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 9, S("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 9, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 18, S(""), 18);
    test(S("abcdefghijklmnopqrst"), 1, 18, S("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 18, S("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 18, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 19, S(""), 19);
    test(S("abcdefghijklmnopqrst"), 1, 19, S("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 19, S("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 19, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 20, S(""), 19);
    test(S("abcdefghijklmnopqrst"), 1, 20, S("abcde"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 20, S("abcdefghij"), 1);
    test(S("abcdefghijklmnopqrst"), 1, 20, S("abcdefghijklmnopqrst"), 1);
    test(S("abcdefghijklmnopqrst"), 10, 0, S(""), 0);
    test(S("abcdefghijklmnopqrst"), 10, 0, S("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 10, 0, S("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 10, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 10, 1, S(""), 1);
    test(S("abcdefghijklmnopqrst"), 10, 1, S("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 1, S("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 1, S("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 5, S(""), 5);
    test(S("abcdefghijklmnopqrst"), 10, 5, S("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 5, S("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 5, S("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 9, S(""), 9);
    test(S("abcdefghijklmnopqrst"), 10, 9, S("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 9, S("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 9, S("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, S(""), 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, S("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, S("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, S("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, S(""), 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, S("abcde"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, S("abcdefghij"), 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, S("abcdefghijklmnopqrst"), 10);
    test(S("abcdefghijklmnopqrst"), 19, 0, S(""), 0);
    test(S("abcdefghijklmnopqrst"), 19, 0, S("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 19, 0, S("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 19, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 19, 1, S(""), 1);
    test(S("abcdefghijklmnopqrst"), 19, 1, S("abcde"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 1, S("abcdefghij"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 1, S("abcdefghijklmnopqrst"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 2, S(""), 1);
    test(S("abcdefghijklmnopqrst"), 19, 2, S("abcde"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 2, S("abcdefghij"), 19);
    test(S("abcdefghijklmnopqrst"), 19, 2, S("abcdefghijklmnopqrst"), 19);
    test(S("abcdefghijklmnopqrst"), 20, 0, S(""), 0);
    test(S("abcdefghijklmnopqrst"), 20, 0, S("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 20, 0, S("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 20, 0, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 20, 1, S(""), 0);
    test(S("abcdefghijklmnopqrst"), 20, 1, S("abcde"), -5);
    test(S("abcdefghijklmnopqrst"), 20, 1, S("abcdefghij"), -10);
    test(S("abcdefghijklmnopqrst"), 20, 1, S("abcdefghijklmnopqrst"), -20);
    test(S("abcdefghijklmnopqrst"), 21, 0, S(""), 0);
    test(S("abcdefghijklmnopqrst"), 21, 0, S("abcde"), 0);
    test(S("abcdefghijklmnopqrst"), 21, 0, S("abcdefghij"), 0);
    test(S("abcdefghijklmnopqrst"), 21, 0, S("abcdefghijklmnopqrst"), 0);
}

int main()
{
    {
    typedef std::string S;
    test0<S>();
    test1<S>();
    test2<S>();
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test0<S>();
    test1<S>();
    test2<S>();
    }
#endif

#if TEST_STD_VER > 3
    {   // LWG 2946
    std::string s = " !";
    assert(s.compare(0, 1, {"abc", 1}) < 0);
    }
#endif
}
