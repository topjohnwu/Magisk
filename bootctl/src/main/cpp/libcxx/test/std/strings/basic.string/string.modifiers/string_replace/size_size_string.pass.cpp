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
//   replace(size_type pos1, size_type n1, const basic_string<charT,traits,Allocator>& str);

#include <string>
#include <stdexcept>
#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, typename S::size_type pos1, typename S::size_type n1, S str, S expected)
{
    const typename S::size_type old_size = s.size();
    S s0 = s;
    if (pos1 <= old_size)
    {
        s.replace(pos1, n1, str);
        LIBCPP_ASSERT(s.__invariants());
        assert(s == expected);
        typename S::size_type xlen = std::min(n1, old_size - pos1);
        typename S::size_type rlen = str.size();
        assert(s.size() == old_size - xlen + rlen);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            s.replace(pos1, n1, str);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos1 > old_size);
            assert(s == s0);
        }
    }
#endif
}

template <class S>
void test0()
{
    test(S(""), 0, 0, S(""), S(""));
    test(S(""), 0, 0, S("12345"), S("12345"));
    test(S(""), 0, 0, S("1234567890"), S("1234567890"));
    test(S(""), 0, 0, S("12345678901234567890"), S("12345678901234567890"));
    test(S(""), 0, 1, S(""), S(""));
    test(S(""), 0, 1, S("12345"), S("12345"));
    test(S(""), 0, 1, S("1234567890"), S("1234567890"));
    test(S(""), 0, 1, S("12345678901234567890"), S("12345678901234567890"));
    test(S(""), 1, 0, S(""), S("can't happen"));
    test(S(""), 1, 0, S("12345"), S("can't happen"));
    test(S(""), 1, 0, S("1234567890"), S("can't happen"));
    test(S(""), 1, 0, S("12345678901234567890"), S("can't happen"));
    test(S("abcde"), 0, 0, S(""), S("abcde"));
    test(S("abcde"), 0, 0, S("12345"), S("12345abcde"));
    test(S("abcde"), 0, 0, S("1234567890"), S("1234567890abcde"));
    test(S("abcde"), 0, 0, S("12345678901234567890"), S("12345678901234567890abcde"));
    test(S("abcde"), 0, 1, S(""), S("bcde"));
    test(S("abcde"), 0, 1, S("12345"), S("12345bcde"));
    test(S("abcde"), 0, 1, S("1234567890"), S("1234567890bcde"));
    test(S("abcde"), 0, 1, S("12345678901234567890"), S("12345678901234567890bcde"));
    test(S("abcde"), 0, 2, S(""), S("cde"));
    test(S("abcde"), 0, 2, S("12345"), S("12345cde"));
    test(S("abcde"), 0, 2, S("1234567890"), S("1234567890cde"));
    test(S("abcde"), 0, 2, S("12345678901234567890"), S("12345678901234567890cde"));
    test(S("abcde"), 0, 4, S(""), S("e"));
    test(S("abcde"), 0, 4, S("12345"), S("12345e"));
    test(S("abcde"), 0, 4, S("1234567890"), S("1234567890e"));
    test(S("abcde"), 0, 4, S("12345678901234567890"), S("12345678901234567890e"));
    test(S("abcde"), 0, 5, S(""), S(""));
    test(S("abcde"), 0, 5, S("12345"), S("12345"));
    test(S("abcde"), 0, 5, S("1234567890"), S("1234567890"));
    test(S("abcde"), 0, 5, S("12345678901234567890"), S("12345678901234567890"));
    test(S("abcde"), 0, 6, S(""), S(""));
    test(S("abcde"), 0, 6, S("12345"), S("12345"));
    test(S("abcde"), 0, 6, S("1234567890"), S("1234567890"));
    test(S("abcde"), 0, 6, S("12345678901234567890"), S("12345678901234567890"));
    test(S("abcde"), 1, 0, S(""), S("abcde"));
    test(S("abcde"), 1, 0, S("12345"), S("a12345bcde"));
    test(S("abcde"), 1, 0, S("1234567890"), S("a1234567890bcde"));
    test(S("abcde"), 1, 0, S("12345678901234567890"), S("a12345678901234567890bcde"));
    test(S("abcde"), 1, 1, S(""), S("acde"));
    test(S("abcde"), 1, 1, S("12345"), S("a12345cde"));
    test(S("abcde"), 1, 1, S("1234567890"), S("a1234567890cde"));
    test(S("abcde"), 1, 1, S("12345678901234567890"), S("a12345678901234567890cde"));
    test(S("abcde"), 1, 2, S(""), S("ade"));
    test(S("abcde"), 1, 2, S("12345"), S("a12345de"));
    test(S("abcde"), 1, 2, S("1234567890"), S("a1234567890de"));
    test(S("abcde"), 1, 2, S("12345678901234567890"), S("a12345678901234567890de"));
    test(S("abcde"), 1, 3, S(""), S("ae"));
    test(S("abcde"), 1, 3, S("12345"), S("a12345e"));
    test(S("abcde"), 1, 3, S("1234567890"), S("a1234567890e"));
    test(S("abcde"), 1, 3, S("12345678901234567890"), S("a12345678901234567890e"));
    test(S("abcde"), 1, 4, S(""), S("a"));
    test(S("abcde"), 1, 4, S("12345"), S("a12345"));
    test(S("abcde"), 1, 4, S("1234567890"), S("a1234567890"));
    test(S("abcde"), 1, 4, S("12345678901234567890"), S("a12345678901234567890"));
    test(S("abcde"), 1, 5, S(""), S("a"));
    test(S("abcde"), 1, 5, S("12345"), S("a12345"));
    test(S("abcde"), 1, 5, S("1234567890"), S("a1234567890"));
    test(S("abcde"), 1, 5, S("12345678901234567890"), S("a12345678901234567890"));
    test(S("abcde"), 2, 0, S(""), S("abcde"));
    test(S("abcde"), 2, 0, S("12345"), S("ab12345cde"));
    test(S("abcde"), 2, 0, S("1234567890"), S("ab1234567890cde"));
    test(S("abcde"), 2, 0, S("12345678901234567890"), S("ab12345678901234567890cde"));
    test(S("abcde"), 2, 1, S(""), S("abde"));
    test(S("abcde"), 2, 1, S("12345"), S("ab12345de"));
    test(S("abcde"), 2, 1, S("1234567890"), S("ab1234567890de"));
    test(S("abcde"), 2, 1, S("12345678901234567890"), S("ab12345678901234567890de"));
    test(S("abcde"), 2, 2, S(""), S("abe"));
    test(S("abcde"), 2, 2, S("12345"), S("ab12345e"));
    test(S("abcde"), 2, 2, S("1234567890"), S("ab1234567890e"));
    test(S("abcde"), 2, 2, S("12345678901234567890"), S("ab12345678901234567890e"));
    test(S("abcde"), 2, 3, S(""), S("ab"));
    test(S("abcde"), 2, 3, S("12345"), S("ab12345"));
    test(S("abcde"), 2, 3, S("1234567890"), S("ab1234567890"));
    test(S("abcde"), 2, 3, S("12345678901234567890"), S("ab12345678901234567890"));
    test(S("abcde"), 2, 4, S(""), S("ab"));
    test(S("abcde"), 2, 4, S("12345"), S("ab12345"));
    test(S("abcde"), 2, 4, S("1234567890"), S("ab1234567890"));
    test(S("abcde"), 2, 4, S("12345678901234567890"), S("ab12345678901234567890"));
    test(S("abcde"), 4, 0, S(""), S("abcde"));
    test(S("abcde"), 4, 0, S("12345"), S("abcd12345e"));
    test(S("abcde"), 4, 0, S("1234567890"), S("abcd1234567890e"));
    test(S("abcde"), 4, 0, S("12345678901234567890"), S("abcd12345678901234567890e"));
    test(S("abcde"), 4, 1, S(""), S("abcd"));
    test(S("abcde"), 4, 1, S("12345"), S("abcd12345"));
    test(S("abcde"), 4, 1, S("1234567890"), S("abcd1234567890"));
    test(S("abcde"), 4, 1, S("12345678901234567890"), S("abcd12345678901234567890"));
    test(S("abcde"), 4, 2, S(""), S("abcd"));
    test(S("abcde"), 4, 2, S("12345"), S("abcd12345"));
    test(S("abcde"), 4, 2, S("1234567890"), S("abcd1234567890"));
    test(S("abcde"), 4, 2, S("12345678901234567890"), S("abcd12345678901234567890"));
    test(S("abcde"), 5, 0, S(""), S("abcde"));
    test(S("abcde"), 5, 0, S("12345"), S("abcde12345"));
    test(S("abcde"), 5, 0, S("1234567890"), S("abcde1234567890"));
    test(S("abcde"), 5, 0, S("12345678901234567890"), S("abcde12345678901234567890"));
    test(S("abcde"), 5, 1, S(""), S("abcde"));
    test(S("abcde"), 5, 1, S("12345"), S("abcde12345"));
    test(S("abcde"), 5, 1, S("1234567890"), S("abcde1234567890"));
    test(S("abcde"), 5, 1, S("12345678901234567890"), S("abcde12345678901234567890"));
}

template <class S>
void test1()
{
    test(S("abcde"), 6, 0, S(""), S("can't happen"));
    test(S("abcde"), 6, 0, S("12345"), S("can't happen"));
    test(S("abcde"), 6, 0, S("1234567890"), S("can't happen"));
    test(S("abcde"), 6, 0, S("12345678901234567890"), S("can't happen"));
    test(S("abcdefghij"), 0, 0, S(""), S("abcdefghij"));
    test(S("abcdefghij"), 0, 0, S("12345"), S("12345abcdefghij"));
    test(S("abcdefghij"), 0, 0, S("1234567890"), S("1234567890abcdefghij"));
    test(S("abcdefghij"), 0, 0, S("12345678901234567890"), S("12345678901234567890abcdefghij"));
    test(S("abcdefghij"), 0, 1, S(""), S("bcdefghij"));
    test(S("abcdefghij"), 0, 1, S("12345"), S("12345bcdefghij"));
    test(S("abcdefghij"), 0, 1, S("1234567890"), S("1234567890bcdefghij"));
    test(S("abcdefghij"), 0, 1, S("12345678901234567890"), S("12345678901234567890bcdefghij"));
    test(S("abcdefghij"), 0, 5, S(""), S("fghij"));
    test(S("abcdefghij"), 0, 5, S("12345"), S("12345fghij"));
    test(S("abcdefghij"), 0, 5, S("1234567890"), S("1234567890fghij"));
    test(S("abcdefghij"), 0, 5, S("12345678901234567890"), S("12345678901234567890fghij"));
    test(S("abcdefghij"), 0, 9, S(""), S("j"));
    test(S("abcdefghij"), 0, 9, S("12345"), S("12345j"));
    test(S("abcdefghij"), 0, 9, S("1234567890"), S("1234567890j"));
    test(S("abcdefghij"), 0, 9, S("12345678901234567890"), S("12345678901234567890j"));
    test(S("abcdefghij"), 0, 10, S(""), S(""));
    test(S("abcdefghij"), 0, 10, S("12345"), S("12345"));
    test(S("abcdefghij"), 0, 10, S("1234567890"), S("1234567890"));
    test(S("abcdefghij"), 0, 10, S("12345678901234567890"), S("12345678901234567890"));
    test(S("abcdefghij"), 0, 11, S(""), S(""));
    test(S("abcdefghij"), 0, 11, S("12345"), S("12345"));
    test(S("abcdefghij"), 0, 11, S("1234567890"), S("1234567890"));
    test(S("abcdefghij"), 0, 11, S("12345678901234567890"), S("12345678901234567890"));
    test(S("abcdefghij"), 1, 0, S(""), S("abcdefghij"));
    test(S("abcdefghij"), 1, 0, S("12345"), S("a12345bcdefghij"));
    test(S("abcdefghij"), 1, 0, S("1234567890"), S("a1234567890bcdefghij"));
    test(S("abcdefghij"), 1, 0, S("12345678901234567890"), S("a12345678901234567890bcdefghij"));
    test(S("abcdefghij"), 1, 1, S(""), S("acdefghij"));
    test(S("abcdefghij"), 1, 1, S("12345"), S("a12345cdefghij"));
    test(S("abcdefghij"), 1, 1, S("1234567890"), S("a1234567890cdefghij"));
    test(S("abcdefghij"), 1, 1, S("12345678901234567890"), S("a12345678901234567890cdefghij"));
    test(S("abcdefghij"), 1, 4, S(""), S("afghij"));
    test(S("abcdefghij"), 1, 4, S("12345"), S("a12345fghij"));
    test(S("abcdefghij"), 1, 4, S("1234567890"), S("a1234567890fghij"));
    test(S("abcdefghij"), 1, 4, S("12345678901234567890"), S("a12345678901234567890fghij"));
    test(S("abcdefghij"), 1, 8, S(""), S("aj"));
    test(S("abcdefghij"), 1, 8, S("12345"), S("a12345j"));
    test(S("abcdefghij"), 1, 8, S("1234567890"), S("a1234567890j"));
    test(S("abcdefghij"), 1, 8, S("12345678901234567890"), S("a12345678901234567890j"));
    test(S("abcdefghij"), 1, 9, S(""), S("a"));
    test(S("abcdefghij"), 1, 9, S("12345"), S("a12345"));
    test(S("abcdefghij"), 1, 9, S("1234567890"), S("a1234567890"));
    test(S("abcdefghij"), 1, 9, S("12345678901234567890"), S("a12345678901234567890"));
    test(S("abcdefghij"), 1, 10, S(""), S("a"));
    test(S("abcdefghij"), 1, 10, S("12345"), S("a12345"));
    test(S("abcdefghij"), 1, 10, S("1234567890"), S("a1234567890"));
    test(S("abcdefghij"), 1, 10, S("12345678901234567890"), S("a12345678901234567890"));
    test(S("abcdefghij"), 5, 0, S(""), S("abcdefghij"));
    test(S("abcdefghij"), 5, 0, S("12345"), S("abcde12345fghij"));
    test(S("abcdefghij"), 5, 0, S("1234567890"), S("abcde1234567890fghij"));
    test(S("abcdefghij"), 5, 0, S("12345678901234567890"), S("abcde12345678901234567890fghij"));
    test(S("abcdefghij"), 5, 1, S(""), S("abcdeghij"));
    test(S("abcdefghij"), 5, 1, S("12345"), S("abcde12345ghij"));
    test(S("abcdefghij"), 5, 1, S("1234567890"), S("abcde1234567890ghij"));
    test(S("abcdefghij"), 5, 1, S("12345678901234567890"), S("abcde12345678901234567890ghij"));
    test(S("abcdefghij"), 5, 2, S(""), S("abcdehij"));
    test(S("abcdefghij"), 5, 2, S("12345"), S("abcde12345hij"));
    test(S("abcdefghij"), 5, 2, S("1234567890"), S("abcde1234567890hij"));
    test(S("abcdefghij"), 5, 2, S("12345678901234567890"), S("abcde12345678901234567890hij"));
    test(S("abcdefghij"), 5, 4, S(""), S("abcdej"));
    test(S("abcdefghij"), 5, 4, S("12345"), S("abcde12345j"));
    test(S("abcdefghij"), 5, 4, S("1234567890"), S("abcde1234567890j"));
    test(S("abcdefghij"), 5, 4, S("12345678901234567890"), S("abcde12345678901234567890j"));
    test(S("abcdefghij"), 5, 5, S(""), S("abcde"));
    test(S("abcdefghij"), 5, 5, S("12345"), S("abcde12345"));
    test(S("abcdefghij"), 5, 5, S("1234567890"), S("abcde1234567890"));
    test(S("abcdefghij"), 5, 5, S("12345678901234567890"), S("abcde12345678901234567890"));
    test(S("abcdefghij"), 5, 6, S(""), S("abcde"));
    test(S("abcdefghij"), 5, 6, S("12345"), S("abcde12345"));
    test(S("abcdefghij"), 5, 6, S("1234567890"), S("abcde1234567890"));
    test(S("abcdefghij"), 5, 6, S("12345678901234567890"), S("abcde12345678901234567890"));
    test(S("abcdefghij"), 9, 0, S(""), S("abcdefghij"));
    test(S("abcdefghij"), 9, 0, S("12345"), S("abcdefghi12345j"));
    test(S("abcdefghij"), 9, 0, S("1234567890"), S("abcdefghi1234567890j"));
    test(S("abcdefghij"), 9, 0, S("12345678901234567890"), S("abcdefghi12345678901234567890j"));
    test(S("abcdefghij"), 9, 1, S(""), S("abcdefghi"));
    test(S("abcdefghij"), 9, 1, S("12345"), S("abcdefghi12345"));
    test(S("abcdefghij"), 9, 1, S("1234567890"), S("abcdefghi1234567890"));
    test(S("abcdefghij"), 9, 1, S("12345678901234567890"), S("abcdefghi12345678901234567890"));
    test(S("abcdefghij"), 9, 2, S(""), S("abcdefghi"));
    test(S("abcdefghij"), 9, 2, S("12345"), S("abcdefghi12345"));
    test(S("abcdefghij"), 9, 2, S("1234567890"), S("abcdefghi1234567890"));
    test(S("abcdefghij"), 9, 2, S("12345678901234567890"), S("abcdefghi12345678901234567890"));
    test(S("abcdefghij"), 10, 0, S(""), S("abcdefghij"));
    test(S("abcdefghij"), 10, 0, S("12345"), S("abcdefghij12345"));
    test(S("abcdefghij"), 10, 0, S("1234567890"), S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, 0, S("12345678901234567890"), S("abcdefghij12345678901234567890"));
    test(S("abcdefghij"), 10, 1, S(""), S("abcdefghij"));
    test(S("abcdefghij"), 10, 1, S("12345"), S("abcdefghij12345"));
    test(S("abcdefghij"), 10, 1, S("1234567890"), S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, 1, S("12345678901234567890"), S("abcdefghij12345678901234567890"));
    test(S("abcdefghij"), 11, 0, S(""), S("can't happen"));
    test(S("abcdefghij"), 11, 0, S("12345"), S("can't happen"));
    test(S("abcdefghij"), 11, 0, S("1234567890"), S("can't happen"));
    test(S("abcdefghij"), 11, 0, S("12345678901234567890"), S("can't happen"));
}

template <class S>
void test2()
{
    test(S("abcdefghijklmnopqrst"), 0, 0, S(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, S("12345"), S("12345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, S("1234567890"), S("1234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, S("12345678901234567890"), S("12345678901234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, S(""), S("bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, S("12345"), S("12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, S("1234567890"), S("1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, S("12345678901234567890"), S("12345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, S(""), S("klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, S("12345"), S("12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, S("1234567890"), S("1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, S("12345678901234567890"), S("12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 19, S(""), S("t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, S("12345"), S("12345t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, S("1234567890"), S("1234567890t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, S("12345678901234567890"), S("12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 0, 20, S(""), S(""));
    test(S("abcdefghijklmnopqrst"), 0, 20, S("12345"), S("12345"));
    test(S("abcdefghijklmnopqrst"), 0, 20, S("1234567890"), S("1234567890"));
    test(S("abcdefghijklmnopqrst"), 0, 20, S("12345678901234567890"), S("12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 0, 21, S(""), S(""));
    test(S("abcdefghijklmnopqrst"), 0, 21, S("12345"), S("12345"));
    test(S("abcdefghijklmnopqrst"), 0, 21, S("1234567890"), S("1234567890"));
    test(S("abcdefghijklmnopqrst"), 0, 21, S("12345678901234567890"), S("12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 1, 0, S(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, S("12345"), S("a12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, S("1234567890"), S("a1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, S("12345678901234567890"), S("a12345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, S(""), S("acdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, S("12345"), S("a12345cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, S("1234567890"), S("a1234567890cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, S("12345678901234567890"), S("a12345678901234567890cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, S(""), S("aklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, S("12345"), S("a12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, S("1234567890"), S("a1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, S("12345678901234567890"), S("a12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 18, S(""), S("at"));
    test(S("abcdefghijklmnopqrst"), 1, 18, S("12345"), S("a12345t"));
    test(S("abcdefghijklmnopqrst"), 1, 18, S("1234567890"), S("a1234567890t"));
    test(S("abcdefghijklmnopqrst"), 1, 18, S("12345678901234567890"), S("a12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 1, 19, S(""), S("a"));
    test(S("abcdefghijklmnopqrst"), 1, 19, S("12345"), S("a12345"));
    test(S("abcdefghijklmnopqrst"), 1, 19, S("1234567890"), S("a1234567890"));
    test(S("abcdefghijklmnopqrst"), 1, 19, S("12345678901234567890"), S("a12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 1, 20, S(""), S("a"));
    test(S("abcdefghijklmnopqrst"), 1, 20, S("12345"), S("a12345"));
    test(S("abcdefghijklmnopqrst"), 1, 20, S("1234567890"), S("a1234567890"));
    test(S("abcdefghijklmnopqrst"), 1, 20, S("12345678901234567890"), S("a12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 10, 0, S(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, S("12345"), S("abcdefghij12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, S("1234567890"), S("abcdefghij1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, S("12345678901234567890"), S("abcdefghij12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, S(""), S("abcdefghijlmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, S("12345"), S("abcdefghij12345lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, S("1234567890"), S("abcdefghij1234567890lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, S("12345678901234567890"), S("abcdefghij12345678901234567890lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, S(""), S("abcdefghijpqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, S("12345"), S("abcdefghij12345pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, S("1234567890"), S("abcdefghij1234567890pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, S("12345678901234567890"), S("abcdefghij12345678901234567890pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 9, S(""), S("abcdefghijt"));
    test(S("abcdefghijklmnopqrst"), 10, 9, S("12345"), S("abcdefghij12345t"));
    test(S("abcdefghijklmnopqrst"), 10, 9, S("1234567890"), S("abcdefghij1234567890t"));
    test(S("abcdefghijklmnopqrst"), 10, 9, S("12345678901234567890"), S("abcdefghij12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 10, 10, S(""), S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 10, 10, S("12345"), S("abcdefghij12345"));
    test(S("abcdefghijklmnopqrst"), 10, 10, S("1234567890"), S("abcdefghij1234567890"));
    test(S("abcdefghijklmnopqrst"), 10, 10, S("12345678901234567890"), S("abcdefghij12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 10, 11, S(""), S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 10, 11, S("12345"), S("abcdefghij12345"));
    test(S("abcdefghijklmnopqrst"), 10, 11, S("1234567890"), S("abcdefghij1234567890"));
    test(S("abcdefghijklmnopqrst"), 10, 11, S("12345678901234567890"), S("abcdefghij12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 19, 0, S(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 0, S("12345"), S("abcdefghijklmnopqrs12345t"));
    test(S("abcdefghijklmnopqrst"), 19, 0, S("1234567890"), S("abcdefghijklmnopqrs1234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, 0, S("12345678901234567890"), S("abcdefghijklmnopqrs12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, 1, S(""), S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 19, 1, S("12345"), S("abcdefghijklmnopqrs12345"));
    test(S("abcdefghijklmnopqrst"), 19, 1, S("1234567890"), S("abcdefghijklmnopqrs1234567890"));
    test(S("abcdefghijklmnopqrst"), 19, 1, S("12345678901234567890"), S("abcdefghijklmnopqrs12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 19, 2, S(""), S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 19, 2, S("12345"), S("abcdefghijklmnopqrs12345"));
    test(S("abcdefghijklmnopqrst"), 19, 2, S("1234567890"), S("abcdefghijklmnopqrs1234567890"));
    test(S("abcdefghijklmnopqrst"), 19, 2, S("12345678901234567890"), S("abcdefghijklmnopqrs12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 20, 0, S(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 0, S("12345"), S("abcdefghijklmnopqrst12345"));
    test(S("abcdefghijklmnopqrst"), 20, 0, S("1234567890"), S("abcdefghijklmnopqrst1234567890"));
    test(S("abcdefghijklmnopqrst"), 20, 0, S("12345678901234567890"), S("abcdefghijklmnopqrst12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 20, 1, S(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 1, S("12345"), S("abcdefghijklmnopqrst12345"));
    test(S("abcdefghijklmnopqrst"), 20, 1, S("1234567890"), S("abcdefghijklmnopqrst1234567890"));
    test(S("abcdefghijklmnopqrst"), 20, 1, S("12345678901234567890"), S("abcdefghijklmnopqrst12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 21, 0, S(""), S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, 0, S("12345"), S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, 0, S("1234567890"), S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, 0, S("12345678901234567890"), S("can't happen"));
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
    std::string s = " ";
    s.replace(0, 1, {"abc", 1});
    assert(s.size() == 1);
    assert(s == "a");
    }
#endif
}
