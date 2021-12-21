//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// int compare(size_type pos, size_type n1, const charT *s) const;

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

template <class S>
void
test(const S& s, typename S::size_type pos1, typename S::size_type n1,
     const typename S::value_type* str, int x)
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
    test(S(""), 0, 0, "", 0);
    test(S(""), 0, 0, "abcde", -5);
    test(S(""), 0, 0, "abcdefghij", -10);
    test(S(""), 0, 0, "abcdefghijklmnopqrst", -20);
    test(S(""), 0, 1, "", 0);
    test(S(""), 0, 1, "abcde", -5);
    test(S(""), 0, 1, "abcdefghij", -10);
    test(S(""), 0, 1, "abcdefghijklmnopqrst", -20);
    test(S(""), 1, 0, "", 0);
    test(S(""), 1, 0, "abcde", 0);
    test(S(""), 1, 0, "abcdefghij", 0);
    test(S(""), 1, 0, "abcdefghijklmnopqrst", 0);
    test(S("abcde"), 0, 0, "", 0);
    test(S("abcde"), 0, 0, "abcde", -5);
    test(S("abcde"), 0, 0, "abcdefghij", -10);
    test(S("abcde"), 0, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcde"), 0, 1, "", 1);
    test(S("abcde"), 0, 1, "abcde", -4);
    test(S("abcde"), 0, 1, "abcdefghij", -9);
    test(S("abcde"), 0, 1, "abcdefghijklmnopqrst", -19);
    test(S("abcde"), 0, 2, "", 2);
    test(S("abcde"), 0, 2, "abcde", -3);
    test(S("abcde"), 0, 2, "abcdefghij", -8);
    test(S("abcde"), 0, 2, "abcdefghijklmnopqrst", -18);
    test(S("abcde"), 0, 4, "", 4);
    test(S("abcde"), 0, 4, "abcde", -1);
    test(S("abcde"), 0, 4, "abcdefghij", -6);
    test(S("abcde"), 0, 4, "abcdefghijklmnopqrst", -16);
    test(S("abcde"), 0, 5, "", 5);
    test(S("abcde"), 0, 5, "abcde", 0);
    test(S("abcde"), 0, 5, "abcdefghij", -5);
    test(S("abcde"), 0, 5, "abcdefghijklmnopqrst", -15);
    test(S("abcde"), 0, 6, "", 5);
    test(S("abcde"), 0, 6, "abcde", 0);
    test(S("abcde"), 0, 6, "abcdefghij", -5);
    test(S("abcde"), 0, 6, "abcdefghijklmnopqrst", -15);
    test(S("abcde"), 1, 0, "", 0);
    test(S("abcde"), 1, 0, "abcde", -5);
    test(S("abcde"), 1, 0, "abcdefghij", -10);
    test(S("abcde"), 1, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcde"), 1, 1, "", 1);
    test(S("abcde"), 1, 1, "abcde", 1);
    test(S("abcde"), 1, 1, "abcdefghij", 1);
    test(S("abcde"), 1, 1, "abcdefghijklmnopqrst", 1);
    test(S("abcde"), 1, 2, "", 2);
    test(S("abcde"), 1, 2, "abcde", 1);
    test(S("abcde"), 1, 2, "abcdefghij", 1);
    test(S("abcde"), 1, 2, "abcdefghijklmnopqrst", 1);
    test(S("abcde"), 1, 3, "", 3);
    test(S("abcde"), 1, 3, "abcde", 1);
    test(S("abcde"), 1, 3, "abcdefghij", 1);
    test(S("abcde"), 1, 3, "abcdefghijklmnopqrst", 1);
    test(S("abcde"), 1, 4, "", 4);
    test(S("abcde"), 1, 4, "abcde", 1);
    test(S("abcde"), 1, 4, "abcdefghij", 1);
    test(S("abcde"), 1, 4, "abcdefghijklmnopqrst", 1);
    test(S("abcde"), 1, 5, "", 4);
    test(S("abcde"), 1, 5, "abcde", 1);
    test(S("abcde"), 1, 5, "abcdefghij", 1);
    test(S("abcde"), 1, 5, "abcdefghijklmnopqrst", 1);
    test(S("abcde"), 2, 0, "", 0);
    test(S("abcde"), 2, 0, "abcde", -5);
    test(S("abcde"), 2, 0, "abcdefghij", -10);
    test(S("abcde"), 2, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcde"), 2, 1, "", 1);
    test(S("abcde"), 2, 1, "abcde", 2);
    test(S("abcde"), 2, 1, "abcdefghij", 2);
    test(S("abcde"), 2, 1, "abcdefghijklmnopqrst", 2);
    test(S("abcde"), 2, 2, "", 2);
    test(S("abcde"), 2, 2, "abcde", 2);
    test(S("abcde"), 2, 2, "abcdefghij", 2);
    test(S("abcde"), 2, 2, "abcdefghijklmnopqrst", 2);
    test(S("abcde"), 2, 3, "", 3);
    test(S("abcde"), 2, 3, "abcde", 2);
    test(S("abcde"), 2, 3, "abcdefghij", 2);
    test(S("abcde"), 2, 3, "abcdefghijklmnopqrst", 2);
    test(S("abcde"), 2, 4, "", 3);
    test(S("abcde"), 2, 4, "abcde", 2);
    test(S("abcde"), 2, 4, "abcdefghij", 2);
    test(S("abcde"), 2, 4, "abcdefghijklmnopqrst", 2);
    test(S("abcde"), 4, 0, "", 0);
    test(S("abcde"), 4, 0, "abcde", -5);
    test(S("abcde"), 4, 0, "abcdefghij", -10);
    test(S("abcde"), 4, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcde"), 4, 1, "", 1);
    test(S("abcde"), 4, 1, "abcde", 4);
    test(S("abcde"), 4, 1, "abcdefghij", 4);
    test(S("abcde"), 4, 1, "abcdefghijklmnopqrst", 4);
    test(S("abcde"), 4, 2, "", 1);
    test(S("abcde"), 4, 2, "abcde", 4);
    test(S("abcde"), 4, 2, "abcdefghij", 4);
    test(S("abcde"), 4, 2, "abcdefghijklmnopqrst", 4);
    test(S("abcde"), 5, 0, "", 0);
    test(S("abcde"), 5, 0, "abcde", -5);
    test(S("abcde"), 5, 0, "abcdefghij", -10);
    test(S("abcde"), 5, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcde"), 5, 1, "", 0);
    test(S("abcde"), 5, 1, "abcde", -5);
    test(S("abcde"), 5, 1, "abcdefghij", -10);
    test(S("abcde"), 5, 1, "abcdefghijklmnopqrst", -20);
}

template <class S>
void test1()
{
    test(S("abcde"), 6, 0, "", 0);
    test(S("abcde"), 6, 0, "abcde", 0);
    test(S("abcde"), 6, 0, "abcdefghij", 0);
    test(S("abcde"), 6, 0, "abcdefghijklmnopqrst", 0);
    test(S("abcdefghij"), 0, 0, "", 0);
    test(S("abcdefghij"), 0, 0, "abcde", -5);
    test(S("abcdefghij"), 0, 0, "abcdefghij", -10);
    test(S("abcdefghij"), 0, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghij"), 0, 1, "", 1);
    test(S("abcdefghij"), 0, 1, "abcde", -4);
    test(S("abcdefghij"), 0, 1, "abcdefghij", -9);
    test(S("abcdefghij"), 0, 1, "abcdefghijklmnopqrst", -19);
    test(S("abcdefghij"), 0, 5, "", 5);
    test(S("abcdefghij"), 0, 5, "abcde", 0);
    test(S("abcdefghij"), 0, 5, "abcdefghij", -5);
    test(S("abcdefghij"), 0, 5, "abcdefghijklmnopqrst", -15);
    test(S("abcdefghij"), 0, 9, "", 9);
    test(S("abcdefghij"), 0, 9, "abcde", 4);
    test(S("abcdefghij"), 0, 9, "abcdefghij", -1);
    test(S("abcdefghij"), 0, 9, "abcdefghijklmnopqrst", -11);
    test(S("abcdefghij"), 0, 10, "", 10);
    test(S("abcdefghij"), 0, 10, "abcde", 5);
    test(S("abcdefghij"), 0, 10, "abcdefghij", 0);
    test(S("abcdefghij"), 0, 10, "abcdefghijklmnopqrst", -10);
    test(S("abcdefghij"), 0, 11, "", 10);
    test(S("abcdefghij"), 0, 11, "abcde", 5);
    test(S("abcdefghij"), 0, 11, "abcdefghij", 0);
    test(S("abcdefghij"), 0, 11, "abcdefghijklmnopqrst", -10);
    test(S("abcdefghij"), 1, 0, "", 0);
    test(S("abcdefghij"), 1, 0, "abcde", -5);
    test(S("abcdefghij"), 1, 0, "abcdefghij", -10);
    test(S("abcdefghij"), 1, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghij"), 1, 1, "", 1);
    test(S("abcdefghij"), 1, 1, "abcde", 1);
    test(S("abcdefghij"), 1, 1, "abcdefghij", 1);
    test(S("abcdefghij"), 1, 1, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghij"), 1, 4, "", 4);
    test(S("abcdefghij"), 1, 4, "abcde", 1);
    test(S("abcdefghij"), 1, 4, "abcdefghij", 1);
    test(S("abcdefghij"), 1, 4, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghij"), 1, 8, "", 8);
    test(S("abcdefghij"), 1, 8, "abcde", 1);
    test(S("abcdefghij"), 1, 8, "abcdefghij", 1);
    test(S("abcdefghij"), 1, 8, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghij"), 1, 9, "", 9);
    test(S("abcdefghij"), 1, 9, "abcde", 1);
    test(S("abcdefghij"), 1, 9, "abcdefghij", 1);
    test(S("abcdefghij"), 1, 9, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghij"), 1, 10, "", 9);
    test(S("abcdefghij"), 1, 10, "abcde", 1);
    test(S("abcdefghij"), 1, 10, "abcdefghij", 1);
    test(S("abcdefghij"), 1, 10, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghij"), 5, 0, "", 0);
    test(S("abcdefghij"), 5, 0, "abcde", -5);
    test(S("abcdefghij"), 5, 0, "abcdefghij", -10);
    test(S("abcdefghij"), 5, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghij"), 5, 1, "", 1);
    test(S("abcdefghij"), 5, 1, "abcde", 5);
    test(S("abcdefghij"), 5, 1, "abcdefghij", 5);
    test(S("abcdefghij"), 5, 1, "abcdefghijklmnopqrst", 5);
    test(S("abcdefghij"), 5, 2, "", 2);
    test(S("abcdefghij"), 5, 2, "abcde", 5);
    test(S("abcdefghij"), 5, 2, "abcdefghij", 5);
    test(S("abcdefghij"), 5, 2, "abcdefghijklmnopqrst", 5);
    test(S("abcdefghij"), 5, 4, "", 4);
    test(S("abcdefghij"), 5, 4, "abcde", 5);
    test(S("abcdefghij"), 5, 4, "abcdefghij", 5);
    test(S("abcdefghij"), 5, 4, "abcdefghijklmnopqrst", 5);
    test(S("abcdefghij"), 5, 5, "", 5);
    test(S("abcdefghij"), 5, 5, "abcde", 5);
    test(S("abcdefghij"), 5, 5, "abcdefghij", 5);
    test(S("abcdefghij"), 5, 5, "abcdefghijklmnopqrst", 5);
    test(S("abcdefghij"), 5, 6, "", 5);
    test(S("abcdefghij"), 5, 6, "abcde", 5);
    test(S("abcdefghij"), 5, 6, "abcdefghij", 5);
    test(S("abcdefghij"), 5, 6, "abcdefghijklmnopqrst", 5);
    test(S("abcdefghij"), 9, 0, "", 0);
    test(S("abcdefghij"), 9, 0, "abcde", -5);
    test(S("abcdefghij"), 9, 0, "abcdefghij", -10);
    test(S("abcdefghij"), 9, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghij"), 9, 1, "", 1);
    test(S("abcdefghij"), 9, 1, "abcde", 9);
    test(S("abcdefghij"), 9, 1, "abcdefghij", 9);
    test(S("abcdefghij"), 9, 1, "abcdefghijklmnopqrst", 9);
    test(S("abcdefghij"), 9, 2, "", 1);
    test(S("abcdefghij"), 9, 2, "abcde", 9);
    test(S("abcdefghij"), 9, 2, "abcdefghij", 9);
    test(S("abcdefghij"), 9, 2, "abcdefghijklmnopqrst", 9);
    test(S("abcdefghij"), 10, 0, "", 0);
    test(S("abcdefghij"), 10, 0, "abcde", -5);
    test(S("abcdefghij"), 10, 0, "abcdefghij", -10);
    test(S("abcdefghij"), 10, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghij"), 10, 1, "", 0);
    test(S("abcdefghij"), 10, 1, "abcde", -5);
    test(S("abcdefghij"), 10, 1, "abcdefghij", -10);
    test(S("abcdefghij"), 10, 1, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghij"), 11, 0, "", 0);
    test(S("abcdefghij"), 11, 0, "abcde", 0);
    test(S("abcdefghij"), 11, 0, "abcdefghij", 0);
    test(S("abcdefghij"), 11, 0, "abcdefghijklmnopqrst", 0);
}

template <class S>
void test2()
{
    test(S("abcdefghijklmnopqrst"), 0, 0, "", 0);
    test(S("abcdefghijklmnopqrst"), 0, 0, "abcde", -5);
    test(S("abcdefghijklmnopqrst"), 0, 0, "abcdefghij", -10);
    test(S("abcdefghijklmnopqrst"), 0, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghijklmnopqrst"), 0, 1, "", 1);
    test(S("abcdefghijklmnopqrst"), 0, 1, "abcde", -4);
    test(S("abcdefghijklmnopqrst"), 0, 1, "abcdefghij", -9);
    test(S("abcdefghijklmnopqrst"), 0, 1, "abcdefghijklmnopqrst", -19);
    test(S("abcdefghijklmnopqrst"), 0, 10, "", 10);
    test(S("abcdefghijklmnopqrst"), 0, 10, "abcde", 5);
    test(S("abcdefghijklmnopqrst"), 0, 10, "abcdefghij", 0);
    test(S("abcdefghijklmnopqrst"), 0, 10, "abcdefghijklmnopqrst", -10);
    test(S("abcdefghijklmnopqrst"), 0, 19, "", 19);
    test(S("abcdefghijklmnopqrst"), 0, 19, "abcde", 14);
    test(S("abcdefghijklmnopqrst"), 0, 19, "abcdefghij", 9);
    test(S("abcdefghijklmnopqrst"), 0, 19, "abcdefghijklmnopqrst", -1);
    test(S("abcdefghijklmnopqrst"), 0, 20, "", 20);
    test(S("abcdefghijklmnopqrst"), 0, 20, "abcde", 15);
    test(S("abcdefghijklmnopqrst"), 0, 20, "abcdefghij", 10);
    test(S("abcdefghijklmnopqrst"), 0, 20, "abcdefghijklmnopqrst", 0);
    test(S("abcdefghijklmnopqrst"), 0, 21, "", 20);
    test(S("abcdefghijklmnopqrst"), 0, 21, "abcde", 15);
    test(S("abcdefghijklmnopqrst"), 0, 21, "abcdefghij", 10);
    test(S("abcdefghijklmnopqrst"), 0, 21, "abcdefghijklmnopqrst", 0);
    test(S("abcdefghijklmnopqrst"), 1, 0, "", 0);
    test(S("abcdefghijklmnopqrst"), 1, 0, "abcde", -5);
    test(S("abcdefghijklmnopqrst"), 1, 0, "abcdefghij", -10);
    test(S("abcdefghijklmnopqrst"), 1, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghijklmnopqrst"), 1, 1, "", 1);
    test(S("abcdefghijklmnopqrst"), 1, 1, "abcde", 1);
    test(S("abcdefghijklmnopqrst"), 1, 1, "abcdefghij", 1);
    test(S("abcdefghijklmnopqrst"), 1, 1, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghijklmnopqrst"), 1, 9, "", 9);
    test(S("abcdefghijklmnopqrst"), 1, 9, "abcde", 1);
    test(S("abcdefghijklmnopqrst"), 1, 9, "abcdefghij", 1);
    test(S("abcdefghijklmnopqrst"), 1, 9, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghijklmnopqrst"), 1, 18, "", 18);
    test(S("abcdefghijklmnopqrst"), 1, 18, "abcde", 1);
    test(S("abcdefghijklmnopqrst"), 1, 18, "abcdefghij", 1);
    test(S("abcdefghijklmnopqrst"), 1, 18, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghijklmnopqrst"), 1, 19, "", 19);
    test(S("abcdefghijklmnopqrst"), 1, 19, "abcde", 1);
    test(S("abcdefghijklmnopqrst"), 1, 19, "abcdefghij", 1);
    test(S("abcdefghijklmnopqrst"), 1, 19, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghijklmnopqrst"), 1, 20, "", 19);
    test(S("abcdefghijklmnopqrst"), 1, 20, "abcde", 1);
    test(S("abcdefghijklmnopqrst"), 1, 20, "abcdefghij", 1);
    test(S("abcdefghijklmnopqrst"), 1, 20, "abcdefghijklmnopqrst", 1);
    test(S("abcdefghijklmnopqrst"), 10, 0, "", 0);
    test(S("abcdefghijklmnopqrst"), 10, 0, "abcde", -5);
    test(S("abcdefghijklmnopqrst"), 10, 0, "abcdefghij", -10);
    test(S("abcdefghijklmnopqrst"), 10, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghijklmnopqrst"), 10, 1, "", 1);
    test(S("abcdefghijklmnopqrst"), 10, 1, "abcde", 10);
    test(S("abcdefghijklmnopqrst"), 10, 1, "abcdefghij", 10);
    test(S("abcdefghijklmnopqrst"), 10, 1, "abcdefghijklmnopqrst", 10);
    test(S("abcdefghijklmnopqrst"), 10, 5, "", 5);
    test(S("abcdefghijklmnopqrst"), 10, 5, "abcde", 10);
    test(S("abcdefghijklmnopqrst"), 10, 5, "abcdefghij", 10);
    test(S("abcdefghijklmnopqrst"), 10, 5, "abcdefghijklmnopqrst", 10);
    test(S("abcdefghijklmnopqrst"), 10, 9, "", 9);
    test(S("abcdefghijklmnopqrst"), 10, 9, "abcde", 10);
    test(S("abcdefghijklmnopqrst"), 10, 9, "abcdefghij", 10);
    test(S("abcdefghijklmnopqrst"), 10, 9, "abcdefghijklmnopqrst", 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, "", 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, "abcde", 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, "abcdefghij", 10);
    test(S("abcdefghijklmnopqrst"), 10, 10, "abcdefghijklmnopqrst", 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, "", 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, "abcde", 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, "abcdefghij", 10);
    test(S("abcdefghijklmnopqrst"), 10, 11, "abcdefghijklmnopqrst", 10);
    test(S("abcdefghijklmnopqrst"), 19, 0, "", 0);
    test(S("abcdefghijklmnopqrst"), 19, 0, "abcde", -5);
    test(S("abcdefghijklmnopqrst"), 19, 0, "abcdefghij", -10);
    test(S("abcdefghijklmnopqrst"), 19, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghijklmnopqrst"), 19, 1, "", 1);
    test(S("abcdefghijklmnopqrst"), 19, 1, "abcde", 19);
    test(S("abcdefghijklmnopqrst"), 19, 1, "abcdefghij", 19);
    test(S("abcdefghijklmnopqrst"), 19, 1, "abcdefghijklmnopqrst", 19);
    test(S("abcdefghijklmnopqrst"), 19, 2, "", 1);
    test(S("abcdefghijklmnopqrst"), 19, 2, "abcde", 19);
    test(S("abcdefghijklmnopqrst"), 19, 2, "abcdefghij", 19);
    test(S("abcdefghijklmnopqrst"), 19, 2, "abcdefghijklmnopqrst", 19);
    test(S("abcdefghijklmnopqrst"), 20, 0, "", 0);
    test(S("abcdefghijklmnopqrst"), 20, 0, "abcde", -5);
    test(S("abcdefghijklmnopqrst"), 20, 0, "abcdefghij", -10);
    test(S("abcdefghijklmnopqrst"), 20, 0, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghijklmnopqrst"), 20, 1, "", 0);
    test(S("abcdefghijklmnopqrst"), 20, 1, "abcde", -5);
    test(S("abcdefghijklmnopqrst"), 20, 1, "abcdefghij", -10);
    test(S("abcdefghijklmnopqrst"), 20, 1, "abcdefghijklmnopqrst", -20);
    test(S("abcdefghijklmnopqrst"), 21, 0, "", 0);
    test(S("abcdefghijklmnopqrst"), 21, 0, "abcde", 0);
    test(S("abcdefghijklmnopqrst"), 21, 0, "abcdefghij", 0);
    test(S("abcdefghijklmnopqrst"), 21, 0, "abcdefghijklmnopqrst", 0);
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
}
