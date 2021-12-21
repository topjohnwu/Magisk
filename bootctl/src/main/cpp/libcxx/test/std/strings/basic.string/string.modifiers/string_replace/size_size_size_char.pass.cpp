//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string<charT,traits,Allocator>&
//   replace(size_type pos, size_type n1, size_type n2, charT c);

#include <string>
#include <stdexcept>
#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, typename S::size_type pos, typename S::size_type n1,
     typename S::size_type n2, typename S::value_type c,
     S expected)
{
    const typename S::size_type old_size = s.size();
    S s0 = s;
    if (pos <= old_size)
    {
        s.replace(pos, n1, n2, c);
        LIBCPP_ASSERT(s.__invariants());
        assert(s == expected);
        typename S::size_type xlen = std::min(n1, old_size - pos);
        typename S::size_type rlen = n2;
        assert(s.size() == old_size - xlen + rlen);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            s.replace(pos, n1, n2, c);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos > old_size);
            assert(s == s0);
        }
    }
#endif
}

template <class S>
void test0()
{
    test(S(""), 0, 0, 0, '2', S(""));
    test(S(""), 0, 0, 5, '2', S("22222"));
    test(S(""), 0, 0, 10, '2', S("2222222222"));
    test(S(""), 0, 0, 20, '2', S("22222222222222222222"));
    test(S(""), 0, 1, 0, '2', S(""));
    test(S(""), 0, 1, 5, '2', S("22222"));
    test(S(""), 0, 1, 10, '2', S("2222222222"));
    test(S(""), 0, 1, 20, '2', S("22222222222222222222"));
    test(S(""), 1, 0, 0, '2', S("can't happen"));
    test(S(""), 1, 0, 5, '2', S("can't happen"));
    test(S(""), 1, 0, 10, '2', S("can't happen"));
    test(S(""), 1, 0, 20, '2', S("can't happen"));
    test(S("abcde"), 0, 0, 0, '2', S("abcde"));
    test(S("abcde"), 0, 0, 5, '2', S("22222abcde"));
    test(S("abcde"), 0, 0, 10, '2', S("2222222222abcde"));
    test(S("abcde"), 0, 0, 20, '2', S("22222222222222222222abcde"));
    test(S("abcde"), 0, 1, 0, '2', S("bcde"));
    test(S("abcde"), 0, 1, 5, '2', S("22222bcde"));
    test(S("abcde"), 0, 1, 10, '2', S("2222222222bcde"));
    test(S("abcde"), 0, 1, 20, '2', S("22222222222222222222bcde"));
    test(S("abcde"), 0, 2, 0, '2', S("cde"));
    test(S("abcde"), 0, 2, 5, '2', S("22222cde"));
    test(S("abcde"), 0, 2, 10, '2', S("2222222222cde"));
    test(S("abcde"), 0, 2, 20, '2', S("22222222222222222222cde"));
    test(S("abcde"), 0, 4, 0, '2', S("e"));
    test(S("abcde"), 0, 4, 5, '2', S("22222e"));
    test(S("abcde"), 0, 4, 10, '2', S("2222222222e"));
    test(S("abcde"), 0, 4, 20, '2', S("22222222222222222222e"));
    test(S("abcde"), 0, 5, 0, '2', S(""));
    test(S("abcde"), 0, 5, 5, '2', S("22222"));
    test(S("abcde"), 0, 5, 10, '2', S("2222222222"));
    test(S("abcde"), 0, 5, 20, '2', S("22222222222222222222"));
    test(S("abcde"), 0, 6, 0, '2', S(""));
    test(S("abcde"), 0, 6, 5, '2', S("22222"));
    test(S("abcde"), 0, 6, 10, '2', S("2222222222"));
    test(S("abcde"), 0, 6, 20, '2', S("22222222222222222222"));
    test(S("abcde"), 1, 0, 0, '2', S("abcde"));
    test(S("abcde"), 1, 0, 5, '2', S("a22222bcde"));
    test(S("abcde"), 1, 0, 10, '2', S("a2222222222bcde"));
    test(S("abcde"), 1, 0, 20, '2', S("a22222222222222222222bcde"));
    test(S("abcde"), 1, 1, 0, '2', S("acde"));
    test(S("abcde"), 1, 1, 5, '2', S("a22222cde"));
    test(S("abcde"), 1, 1, 10, '2', S("a2222222222cde"));
    test(S("abcde"), 1, 1, 20, '2', S("a22222222222222222222cde"));
    test(S("abcde"), 1, 2, 0, '2', S("ade"));
    test(S("abcde"), 1, 2, 5, '2', S("a22222de"));
    test(S("abcde"), 1, 2, 10, '2', S("a2222222222de"));
    test(S("abcde"), 1, 2, 20, '2', S("a22222222222222222222de"));
    test(S("abcde"), 1, 3, 0, '2', S("ae"));
    test(S("abcde"), 1, 3, 5, '2', S("a22222e"));
    test(S("abcde"), 1, 3, 10, '2', S("a2222222222e"));
    test(S("abcde"), 1, 3, 20, '2', S("a22222222222222222222e"));
    test(S("abcde"), 1, 4, 0, '2', S("a"));
    test(S("abcde"), 1, 4, 5, '2', S("a22222"));
    test(S("abcde"), 1, 4, 10, '2', S("a2222222222"));
    test(S("abcde"), 1, 4, 20, '2', S("a22222222222222222222"));
    test(S("abcde"), 1, 5, 0, '2', S("a"));
    test(S("abcde"), 1, 5, 5, '2', S("a22222"));
    test(S("abcde"), 1, 5, 10, '2', S("a2222222222"));
    test(S("abcde"), 1, 5, 20, '2', S("a22222222222222222222"));
    test(S("abcde"), 2, 0, 0, '2', S("abcde"));
    test(S("abcde"), 2, 0, 5, '2', S("ab22222cde"));
    test(S("abcde"), 2, 0, 10, '2', S("ab2222222222cde"));
    test(S("abcde"), 2, 0, 20, '2', S("ab22222222222222222222cde"));
    test(S("abcde"), 2, 1, 0, '2', S("abde"));
    test(S("abcde"), 2, 1, 5, '2', S("ab22222de"));
    test(S("abcde"), 2, 1, 10, '2', S("ab2222222222de"));
    test(S("abcde"), 2, 1, 20, '2', S("ab22222222222222222222de"));
    test(S("abcde"), 2, 2, 0, '2', S("abe"));
    test(S("abcde"), 2, 2, 5, '2', S("ab22222e"));
    test(S("abcde"), 2, 2, 10, '2', S("ab2222222222e"));
    test(S("abcde"), 2, 2, 20, '2', S("ab22222222222222222222e"));
    test(S("abcde"), 2, 3, 0, '2', S("ab"));
    test(S("abcde"), 2, 3, 5, '2', S("ab22222"));
    test(S("abcde"), 2, 3, 10, '2', S("ab2222222222"));
    test(S("abcde"), 2, 3, 20, '2', S("ab22222222222222222222"));
    test(S("abcde"), 2, 4, 0, '2', S("ab"));
    test(S("abcde"), 2, 4, 5, '2', S("ab22222"));
    test(S("abcde"), 2, 4, 10, '2', S("ab2222222222"));
    test(S("abcde"), 2, 4, 20, '2', S("ab22222222222222222222"));
    test(S("abcde"), 4, 0, 0, '2', S("abcde"));
    test(S("abcde"), 4, 0, 5, '2', S("abcd22222e"));
    test(S("abcde"), 4, 0, 10, '2', S("abcd2222222222e"));
    test(S("abcde"), 4, 0, 20, '2', S("abcd22222222222222222222e"));
    test(S("abcde"), 4, 1, 0, '2', S("abcd"));
    test(S("abcde"), 4, 1, 5, '2', S("abcd22222"));
    test(S("abcde"), 4, 1, 10, '2', S("abcd2222222222"));
    test(S("abcde"), 4, 1, 20, '2', S("abcd22222222222222222222"));
    test(S("abcde"), 4, 2, 0, '2', S("abcd"));
    test(S("abcde"), 4, 2, 5, '2', S("abcd22222"));
    test(S("abcde"), 4, 2, 10, '2', S("abcd2222222222"));
    test(S("abcde"), 4, 2, 20, '2', S("abcd22222222222222222222"));
    test(S("abcde"), 5, 0, 0, '2', S("abcde"));
    test(S("abcde"), 5, 0, 5, '2', S("abcde22222"));
    test(S("abcde"), 5, 0, 10, '2', S("abcde2222222222"));
    test(S("abcde"), 5, 0, 20, '2', S("abcde22222222222222222222"));
    test(S("abcde"), 5, 1, 0, '2', S("abcde"));
    test(S("abcde"), 5, 1, 5, '2', S("abcde22222"));
    test(S("abcde"), 5, 1, 10, '2', S("abcde2222222222"));
    test(S("abcde"), 5, 1, 20, '2', S("abcde22222222222222222222"));
}

template <class S>
void test1()
{
    test(S("abcde"), 6, 0, 0, '2', S("can't happen"));
    test(S("abcde"), 6, 0, 5, '2', S("can't happen"));
    test(S("abcde"), 6, 0, 10, '2', S("can't happen"));
    test(S("abcde"), 6, 0, 20, '2', S("can't happen"));
    test(S("abcdefghij"), 0, 0, 0, '2', S("abcdefghij"));
    test(S("abcdefghij"), 0, 0, 5, '2', S("22222abcdefghij"));
    test(S("abcdefghij"), 0, 0, 10, '2', S("2222222222abcdefghij"));
    test(S("abcdefghij"), 0, 0, 20, '2', S("22222222222222222222abcdefghij"));
    test(S("abcdefghij"), 0, 1, 0, '2', S("bcdefghij"));
    test(S("abcdefghij"), 0, 1, 5, '2', S("22222bcdefghij"));
    test(S("abcdefghij"), 0, 1, 10, '2', S("2222222222bcdefghij"));
    test(S("abcdefghij"), 0, 1, 20, '2', S("22222222222222222222bcdefghij"));
    test(S("abcdefghij"), 0, 5, 0, '2', S("fghij"));
    test(S("abcdefghij"), 0, 5, 5, '2', S("22222fghij"));
    test(S("abcdefghij"), 0, 5, 10, '2', S("2222222222fghij"));
    test(S("abcdefghij"), 0, 5, 20, '2', S("22222222222222222222fghij"));
    test(S("abcdefghij"), 0, 9, 0, '2', S("j"));
    test(S("abcdefghij"), 0, 9, 5, '2', S("22222j"));
    test(S("abcdefghij"), 0, 9, 10, '2', S("2222222222j"));
    test(S("abcdefghij"), 0, 9, 20, '2', S("22222222222222222222j"));
    test(S("abcdefghij"), 0, 10, 0, '2', S(""));
    test(S("abcdefghij"), 0, 10, 5, '2', S("22222"));
    test(S("abcdefghij"), 0, 10, 10, '2', S("2222222222"));
    test(S("abcdefghij"), 0, 10, 20, '2', S("22222222222222222222"));
    test(S("abcdefghij"), 0, 11, 0, '2', S(""));
    test(S("abcdefghij"), 0, 11, 5, '2', S("22222"));
    test(S("abcdefghij"), 0, 11, 10, '2', S("2222222222"));
    test(S("abcdefghij"), 0, 11, 20, '2', S("22222222222222222222"));
    test(S("abcdefghij"), 1, 0, 0, '2', S("abcdefghij"));
    test(S("abcdefghij"), 1, 0, 5, '2', S("a22222bcdefghij"));
    test(S("abcdefghij"), 1, 0, 10, '2', S("a2222222222bcdefghij"));
    test(S("abcdefghij"), 1, 0, 20, '2', S("a22222222222222222222bcdefghij"));
    test(S("abcdefghij"), 1, 1, 0, '2', S("acdefghij"));
    test(S("abcdefghij"), 1, 1, 5, '2', S("a22222cdefghij"));
    test(S("abcdefghij"), 1, 1, 10, '2', S("a2222222222cdefghij"));
    test(S("abcdefghij"), 1, 1, 20, '2', S("a22222222222222222222cdefghij"));
    test(S("abcdefghij"), 1, 4, 0, '2', S("afghij"));
    test(S("abcdefghij"), 1, 4, 5, '2', S("a22222fghij"));
    test(S("abcdefghij"), 1, 4, 10, '2', S("a2222222222fghij"));
    test(S("abcdefghij"), 1, 4, 20, '2', S("a22222222222222222222fghij"));
    test(S("abcdefghij"), 1, 8, 0, '2', S("aj"));
    test(S("abcdefghij"), 1, 8, 5, '2', S("a22222j"));
    test(S("abcdefghij"), 1, 8, 10, '2', S("a2222222222j"));
    test(S("abcdefghij"), 1, 8, 20, '2', S("a22222222222222222222j"));
    test(S("abcdefghij"), 1, 9, 0, '2', S("a"));
    test(S("abcdefghij"), 1, 9, 5, '2', S("a22222"));
    test(S("abcdefghij"), 1, 9, 10, '2', S("a2222222222"));
    test(S("abcdefghij"), 1, 9, 20, '2', S("a22222222222222222222"));
    test(S("abcdefghij"), 1, 10, 0, '2', S("a"));
    test(S("abcdefghij"), 1, 10, 5, '2', S("a22222"));
    test(S("abcdefghij"), 1, 10, 10, '2', S("a2222222222"));
    test(S("abcdefghij"), 1, 10, 20, '2', S("a22222222222222222222"));
    test(S("abcdefghij"), 5, 0, 0, '2', S("abcdefghij"));
    test(S("abcdefghij"), 5, 0, 5, '2', S("abcde22222fghij"));
    test(S("abcdefghij"), 5, 0, 10, '2', S("abcde2222222222fghij"));
    test(S("abcdefghij"), 5, 0, 20, '2', S("abcde22222222222222222222fghij"));
    test(S("abcdefghij"), 5, 1, 0, '2', S("abcdeghij"));
    test(S("abcdefghij"), 5, 1, 5, '2', S("abcde22222ghij"));
    test(S("abcdefghij"), 5, 1, 10, '2', S("abcde2222222222ghij"));
    test(S("abcdefghij"), 5, 1, 20, '2', S("abcde22222222222222222222ghij"));
    test(S("abcdefghij"), 5, 2, 0, '2', S("abcdehij"));
    test(S("abcdefghij"), 5, 2, 5, '2', S("abcde22222hij"));
    test(S("abcdefghij"), 5, 2, 10, '2', S("abcde2222222222hij"));
    test(S("abcdefghij"), 5, 2, 20, '2', S("abcde22222222222222222222hij"));
    test(S("abcdefghij"), 5, 4, 0, '2', S("abcdej"));
    test(S("abcdefghij"), 5, 4, 5, '2', S("abcde22222j"));
    test(S("abcdefghij"), 5, 4, 10, '2', S("abcde2222222222j"));
    test(S("abcdefghij"), 5, 4, 20, '2', S("abcde22222222222222222222j"));
    test(S("abcdefghij"), 5, 5, 0, '2', S("abcde"));
    test(S("abcdefghij"), 5, 5, 5, '2', S("abcde22222"));
    test(S("abcdefghij"), 5, 5, 10, '2', S("abcde2222222222"));
    test(S("abcdefghij"), 5, 5, 20, '2', S("abcde22222222222222222222"));
    test(S("abcdefghij"), 5, 6, 0, '2', S("abcde"));
    test(S("abcdefghij"), 5, 6, 5, '2', S("abcde22222"));
    test(S("abcdefghij"), 5, 6, 10, '2', S("abcde2222222222"));
    test(S("abcdefghij"), 5, 6, 20, '2', S("abcde22222222222222222222"));
    test(S("abcdefghij"), 9, 0, 0, '2', S("abcdefghij"));
    test(S("abcdefghij"), 9, 0, 5, '2', S("abcdefghi22222j"));
    test(S("abcdefghij"), 9, 0, 10, '2', S("abcdefghi2222222222j"));
    test(S("abcdefghij"), 9, 0, 20, '2', S("abcdefghi22222222222222222222j"));
    test(S("abcdefghij"), 9, 1, 0, '2', S("abcdefghi"));
    test(S("abcdefghij"), 9, 1, 5, '2', S("abcdefghi22222"));
    test(S("abcdefghij"), 9, 1, 10, '2', S("abcdefghi2222222222"));
    test(S("abcdefghij"), 9, 1, 20, '2', S("abcdefghi22222222222222222222"));
    test(S("abcdefghij"), 9, 2, 0, '2', S("abcdefghi"));
    test(S("abcdefghij"), 9, 2, 5, '2', S("abcdefghi22222"));
    test(S("abcdefghij"), 9, 2, 10, '2', S("abcdefghi2222222222"));
    test(S("abcdefghij"), 9, 2, 20, '2', S("abcdefghi22222222222222222222"));
    test(S("abcdefghij"), 10, 0, 0, '2', S("abcdefghij"));
    test(S("abcdefghij"), 10, 0, 5, '2', S("abcdefghij22222"));
    test(S("abcdefghij"), 10, 0, 10, '2', S("abcdefghij2222222222"));
    test(S("abcdefghij"), 10, 0, 20, '2', S("abcdefghij22222222222222222222"));
    test(S("abcdefghij"), 10, 1, 0, '2', S("abcdefghij"));
    test(S("abcdefghij"), 10, 1, 5, '2', S("abcdefghij22222"));
    test(S("abcdefghij"), 10, 1, 10, '2', S("abcdefghij2222222222"));
    test(S("abcdefghij"), 10, 1, 20, '2', S("abcdefghij22222222222222222222"));
    test(S("abcdefghij"), 11, 0, 0, '2', S("can't happen"));
    test(S("abcdefghij"), 11, 0, 5, '2', S("can't happen"));
    test(S("abcdefghij"), 11, 0, 10, '2', S("can't happen"));
    test(S("abcdefghij"), 11, 0, 20, '2', S("can't happen"));
}

template <class S>
void test2()
{
    test(S("abcdefghijklmnopqrst"), 0, 0, 0, '2', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, 5, '2', S("22222abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, 10, '2', S("2222222222abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, 20, '2', S("22222222222222222222abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, 0, '2', S("bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, 5, '2', S("22222bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, 10, '2', S("2222222222bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, 20, '2', S("22222222222222222222bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, 0, '2', S("klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, 5, '2', S("22222klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, 10, '2', S("2222222222klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, 20, '2', S("22222222222222222222klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 19, 0, '2', S("t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, 5, '2', S("22222t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, 10, '2', S("2222222222t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, 20, '2', S("22222222222222222222t"));
    test(S("abcdefghijklmnopqrst"), 0, 20, 0, '2', S(""));
    test(S("abcdefghijklmnopqrst"), 0, 20, 5, '2', S("22222"));
    test(S("abcdefghijklmnopqrst"), 0, 20, 10, '2', S("2222222222"));
    test(S("abcdefghijklmnopqrst"), 0, 20, 20, '2', S("22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 0, 21, 0, '2', S(""));
    test(S("abcdefghijklmnopqrst"), 0, 21, 5, '2', S("22222"));
    test(S("abcdefghijklmnopqrst"), 0, 21, 10, '2', S("2222222222"));
    test(S("abcdefghijklmnopqrst"), 0, 21, 20, '2', S("22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 1, 0, 0, '2', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, 5, '2', S("a22222bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, 10, '2', S("a2222222222bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, 20, '2', S("a22222222222222222222bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, 0, '2', S("acdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, 5, '2', S("a22222cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, 10, '2', S("a2222222222cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, 20, '2', S("a22222222222222222222cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, 0, '2', S("aklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, 5, '2', S("a22222klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, 10, '2', S("a2222222222klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, 20, '2', S("a22222222222222222222klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 18, 0, '2', S("at"));
    test(S("abcdefghijklmnopqrst"), 1, 18, 5, '2', S("a22222t"));
    test(S("abcdefghijklmnopqrst"), 1, 18, 10, '2', S("a2222222222t"));
    test(S("abcdefghijklmnopqrst"), 1, 18, 20, '2', S("a22222222222222222222t"));
    test(S("abcdefghijklmnopqrst"), 1, 19, 0, '2', S("a"));
    test(S("abcdefghijklmnopqrst"), 1, 19, 5, '2', S("a22222"));
    test(S("abcdefghijklmnopqrst"), 1, 19, 10, '2', S("a2222222222"));
    test(S("abcdefghijklmnopqrst"), 1, 19, 20, '2', S("a22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 1, 20, 0, '2', S("a"));
    test(S("abcdefghijklmnopqrst"), 1, 20, 5, '2', S("a22222"));
    test(S("abcdefghijklmnopqrst"), 1, 20, 10, '2', S("a2222222222"));
    test(S("abcdefghijklmnopqrst"), 1, 20, 20, '2', S("a22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 10, 0, 0, '2', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, 5, '2', S("abcdefghij22222klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, 10, '2', S("abcdefghij2222222222klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, 20, '2', S("abcdefghij22222222222222222222klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, 0, '2', S("abcdefghijlmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, 5, '2', S("abcdefghij22222lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, 10, '2', S("abcdefghij2222222222lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, 20, '2', S("abcdefghij22222222222222222222lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, 0, '2', S("abcdefghijpqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, 5, '2', S("abcdefghij22222pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, 10, '2', S("abcdefghij2222222222pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, 20, '2', S("abcdefghij22222222222222222222pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 9, 0, '2', S("abcdefghijt"));
    test(S("abcdefghijklmnopqrst"), 10, 9, 5, '2', S("abcdefghij22222t"));
    test(S("abcdefghijklmnopqrst"), 10, 9, 10, '2', S("abcdefghij2222222222t"));
    test(S("abcdefghijklmnopqrst"), 10, 9, 20, '2', S("abcdefghij22222222222222222222t"));
    test(S("abcdefghijklmnopqrst"), 10, 10, 0, '2', S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 10, 10, 5, '2', S("abcdefghij22222"));
    test(S("abcdefghijklmnopqrst"), 10, 10, 10, '2', S("abcdefghij2222222222"));
    test(S("abcdefghijklmnopqrst"), 10, 10, 20, '2', S("abcdefghij22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 10, 11, 0, '2', S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 10, 11, 5, '2', S("abcdefghij22222"));
    test(S("abcdefghijklmnopqrst"), 10, 11, 10, '2', S("abcdefghij2222222222"));
    test(S("abcdefghijklmnopqrst"), 10, 11, 20, '2', S("abcdefghij22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 19, 0, 0, '2', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 0, 5, '2', S("abcdefghijklmnopqrs22222t"));
    test(S("abcdefghijklmnopqrst"), 19, 0, 10, '2', S("abcdefghijklmnopqrs2222222222t"));
    test(S("abcdefghijklmnopqrst"), 19, 0, 20, '2', S("abcdefghijklmnopqrs22222222222222222222t"));
    test(S("abcdefghijklmnopqrst"), 19, 1, 0, '2', S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 19, 1, 5, '2', S("abcdefghijklmnopqrs22222"));
    test(S("abcdefghijklmnopqrst"), 19, 1, 10, '2', S("abcdefghijklmnopqrs2222222222"));
    test(S("abcdefghijklmnopqrst"), 19, 1, 20, '2', S("abcdefghijklmnopqrs22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 19, 2, 0, '2', S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 19, 2, 5, '2', S("abcdefghijklmnopqrs22222"));
    test(S("abcdefghijklmnopqrst"), 19, 2, 10, '2', S("abcdefghijklmnopqrs2222222222"));
    test(S("abcdefghijklmnopqrst"), 19, 2, 20, '2', S("abcdefghijklmnopqrs22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 20, 0, 0, '2', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 0, 5, '2', S("abcdefghijklmnopqrst22222"));
    test(S("abcdefghijklmnopqrst"), 20, 0, 10, '2', S("abcdefghijklmnopqrst2222222222"));
    test(S("abcdefghijklmnopqrst"), 20, 0, 20, '2', S("abcdefghijklmnopqrst22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 20, 1, 0, '2', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 1, 5, '2', S("abcdefghijklmnopqrst22222"));
    test(S("abcdefghijklmnopqrst"), 20, 1, 10, '2', S("abcdefghijklmnopqrst2222222222"));
    test(S("abcdefghijklmnopqrst"), 20, 1, 20, '2', S("abcdefghijklmnopqrst22222222222222222222"));
    test(S("abcdefghijklmnopqrst"), 21, 0, 0, '2', S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, 0, 5, '2', S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, 0, 10, '2', S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, 0, 20, '2', S("can't happen"));
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
