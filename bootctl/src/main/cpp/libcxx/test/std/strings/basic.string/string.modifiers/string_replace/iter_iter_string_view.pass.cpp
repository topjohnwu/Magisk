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
//   replace(const_iterator i1, const_iterator i2, basic_string_view<charT,traits> sv);

#include <string>
#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S, class SV>
void
test(S s, typename S::size_type pos1, typename S::size_type n1, SV sv, S expected)
{
    typename S::size_type old_size = s.size();
    typename S::const_iterator first = s.begin() + pos1;
    typename S::const_iterator last = s.begin() + pos1 + n1;
    typename S::size_type xlen = last - first;
    s.replace(first, last, sv);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
    typename S::size_type rlen = sv.size();
    assert(s.size() == old_size - xlen + rlen);
}

template <class S, class SV>
void test0()
{
    test(S(""), 0, 0, SV(""), S(""));
    test(S(""), 0, 0, SV("12345"), S("12345"));
    test(S(""), 0, 0, SV("1234567890"), S("1234567890"));
    test(S(""), 0, 0, SV("12345678901234567890"), S("12345678901234567890"));
    test(S("abcde"), 0, 0, SV(""), S("abcde"));
    test(S("abcde"), 0, 0, SV("12345"), S("12345abcde"));
    test(S("abcde"), 0, 0, SV("1234567890"), S("1234567890abcde"));
    test(S("abcde"), 0, 0, SV("12345678901234567890"), S("12345678901234567890abcde"));
    test(S("abcde"), 0, 1, SV(""), S("bcde"));
    test(S("abcde"), 0, 1, SV("12345"), S("12345bcde"));
    test(S("abcde"), 0, 1, SV("1234567890"), S("1234567890bcde"));
    test(S("abcde"), 0, 1, SV("12345678901234567890"), S("12345678901234567890bcde"));
    test(S("abcde"), 0, 2, SV(""), S("cde"));
    test(S("abcde"), 0, 2, SV("12345"), S("12345cde"));
    test(S("abcde"), 0, 2, SV("1234567890"), S("1234567890cde"));
    test(S("abcde"), 0, 2, SV("12345678901234567890"), S("12345678901234567890cde"));
    test(S("abcde"), 0, 4, SV(""), S("e"));
    test(S("abcde"), 0, 4, SV("12345"), S("12345e"));
    test(S("abcde"), 0, 4, SV("1234567890"), S("1234567890e"));
    test(S("abcde"), 0, 4, SV("12345678901234567890"), S("12345678901234567890e"));
    test(S("abcde"), 0, 5, SV(""), S(""));
    test(S("abcde"), 0, 5, SV("12345"), S("12345"));
    test(S("abcde"), 0, 5, SV("1234567890"), S("1234567890"));
    test(S("abcde"), 0, 5, SV("12345678901234567890"), S("12345678901234567890"));
    test(S("abcde"), 1, 0, SV(""), S("abcde"));
    test(S("abcde"), 1, 0, SV("12345"), S("a12345bcde"));
    test(S("abcde"), 1, 0, SV("1234567890"), S("a1234567890bcde"));
    test(S("abcde"), 1, 0, SV("12345678901234567890"), S("a12345678901234567890bcde"));
    test(S("abcde"), 1, 1, SV(""), S("acde"));
    test(S("abcde"), 1, 1, SV("12345"), S("a12345cde"));
    test(S("abcde"), 1, 1, SV("1234567890"), S("a1234567890cde"));
    test(S("abcde"), 1, 1, SV("12345678901234567890"), S("a12345678901234567890cde"));
    test(S("abcde"), 1, 2, SV(""), S("ade"));
    test(S("abcde"), 1, 2, SV("12345"), S("a12345de"));
    test(S("abcde"), 1, 2, SV("1234567890"), S("a1234567890de"));
    test(S("abcde"), 1, 2, SV("12345678901234567890"), S("a12345678901234567890de"));
    test(S("abcde"), 1, 3, SV(""), S("ae"));
    test(S("abcde"), 1, 3, SV("12345"), S("a12345e"));
    test(S("abcde"), 1, 3, SV("1234567890"), S("a1234567890e"));
    test(S("abcde"), 1, 3, SV("12345678901234567890"), S("a12345678901234567890e"));
    test(S("abcde"), 1, 4, SV(""), S("a"));
    test(S("abcde"), 1, 4, SV("12345"), S("a12345"));
    test(S("abcde"), 1, 4, SV("1234567890"), S("a1234567890"));
    test(S("abcde"), 1, 4, SV("12345678901234567890"), S("a12345678901234567890"));
    test(S("abcde"), 2, 0, SV(""), S("abcde"));
    test(S("abcde"), 2, 0, SV("12345"), S("ab12345cde"));
    test(S("abcde"), 2, 0, SV("1234567890"), S("ab1234567890cde"));
    test(S("abcde"), 2, 0, SV("12345678901234567890"), S("ab12345678901234567890cde"));
    test(S("abcde"), 2, 1, SV(""), S("abde"));
    test(S("abcde"), 2, 1, SV("12345"), S("ab12345de"));
    test(S("abcde"), 2, 1, SV("1234567890"), S("ab1234567890de"));
    test(S("abcde"), 2, 1, SV("12345678901234567890"), S("ab12345678901234567890de"));
    test(S("abcde"), 2, 2, SV(""), S("abe"));
    test(S("abcde"), 2, 2, SV("12345"), S("ab12345e"));
    test(S("abcde"), 2, 2, SV("1234567890"), S("ab1234567890e"));
    test(S("abcde"), 2, 2, SV("12345678901234567890"), S("ab12345678901234567890e"));
    test(S("abcde"), 2, 3, SV(""), S("ab"));
    test(S("abcde"), 2, 3, SV("12345"), S("ab12345"));
    test(S("abcde"), 2, 3, SV("1234567890"), S("ab1234567890"));
    test(S("abcde"), 2, 3, SV("12345678901234567890"), S("ab12345678901234567890"));
    test(S("abcde"), 4, 0, SV(""), S("abcde"));
    test(S("abcde"), 4, 0, SV("12345"), S("abcd12345e"));
    test(S("abcde"), 4, 0, SV("1234567890"), S("abcd1234567890e"));
    test(S("abcde"), 4, 0, SV("12345678901234567890"), S("abcd12345678901234567890e"));
    test(S("abcde"), 4, 1, SV(""), S("abcd"));
    test(S("abcde"), 4, 1, SV("12345"), S("abcd12345"));
    test(S("abcde"), 4, 1, SV("1234567890"), S("abcd1234567890"));
    test(S("abcde"), 4, 1, SV("12345678901234567890"), S("abcd12345678901234567890"));
    test(S("abcde"), 5, 0, SV(""), S("abcde"));
    test(S("abcde"), 5, 0, SV("12345"), S("abcde12345"));
    test(S("abcde"), 5, 0, SV("1234567890"), S("abcde1234567890"));
    test(S("abcde"), 5, 0, SV("12345678901234567890"), S("abcde12345678901234567890"));
    test(S("abcdefghij"), 0, 0, SV(""), S("abcdefghij"));
    test(S("abcdefghij"), 0, 0, SV("12345"), S("12345abcdefghij"));
    test(S("abcdefghij"), 0, 0, SV("1234567890"), S("1234567890abcdefghij"));
    test(S("abcdefghij"), 0, 0, SV("12345678901234567890"), S("12345678901234567890abcdefghij"));
    test(S("abcdefghij"), 0, 1, SV(""), S("bcdefghij"));
    test(S("abcdefghij"), 0, 1, SV("12345"), S("12345bcdefghij"));
    test(S("abcdefghij"), 0, 1, SV("1234567890"), S("1234567890bcdefghij"));
    test(S("abcdefghij"), 0, 1, SV("12345678901234567890"), S("12345678901234567890bcdefghij"));
    test(S("abcdefghij"), 0, 5, SV(""), S("fghij"));
    test(S("abcdefghij"), 0, 5, SV("12345"), S("12345fghij"));
    test(S("abcdefghij"), 0, 5, SV("1234567890"), S("1234567890fghij"));
    test(S("abcdefghij"), 0, 5, SV("12345678901234567890"), S("12345678901234567890fghij"));
    test(S("abcdefghij"), 0, 9, SV(""), S("j"));
    test(S("abcdefghij"), 0, 9, SV("12345"), S("12345j"));
    test(S("abcdefghij"), 0, 9, SV("1234567890"), S("1234567890j"));
    test(S("abcdefghij"), 0, 9, SV("12345678901234567890"), S("12345678901234567890j"));
    test(S("abcdefghij"), 0, 10, SV(""), S(""));
    test(S("abcdefghij"), 0, 10, SV("12345"), S("12345"));
    test(S("abcdefghij"), 0, 10, SV("1234567890"), S("1234567890"));
    test(S("abcdefghij"), 0, 10, SV("12345678901234567890"), S("12345678901234567890"));
    test(S("abcdefghij"), 1, 0, SV(""), S("abcdefghij"));
    test(S("abcdefghij"), 1, 0, SV("12345"), S("a12345bcdefghij"));
    test(S("abcdefghij"), 1, 0, SV("1234567890"), S("a1234567890bcdefghij"));
    test(S("abcdefghij"), 1, 0, SV("12345678901234567890"), S("a12345678901234567890bcdefghij"));
    test(S("abcdefghij"), 1, 1, SV(""), S("acdefghij"));
    test(S("abcdefghij"), 1, 1, SV("12345"), S("a12345cdefghij"));
    test(S("abcdefghij"), 1, 1, SV("1234567890"), S("a1234567890cdefghij"));
    test(S("abcdefghij"), 1, 1, SV("12345678901234567890"), S("a12345678901234567890cdefghij"));
}

template <class S, class SV>
void test1()
{
    test(S("abcdefghij"), 1, 4, SV(""), S("afghij"));
    test(S("abcdefghij"), 1, 4, SV("12345"), S("a12345fghij"));
    test(S("abcdefghij"), 1, 4, SV("1234567890"), S("a1234567890fghij"));
    test(S("abcdefghij"), 1, 4, SV("12345678901234567890"), S("a12345678901234567890fghij"));
    test(S("abcdefghij"), 1, 8, SV(""), S("aj"));
    test(S("abcdefghij"), 1, 8, SV("12345"), S("a12345j"));
    test(S("abcdefghij"), 1, 8, SV("1234567890"), S("a1234567890j"));
    test(S("abcdefghij"), 1, 8, SV("12345678901234567890"), S("a12345678901234567890j"));
    test(S("abcdefghij"), 1, 9, SV(""), S("a"));
    test(S("abcdefghij"), 1, 9, SV("12345"), S("a12345"));
    test(S("abcdefghij"), 1, 9, SV("1234567890"), S("a1234567890"));
    test(S("abcdefghij"), 1, 9, SV("12345678901234567890"), S("a12345678901234567890"));
    test(S("abcdefghij"), 5, 0, SV(""), S("abcdefghij"));
    test(S("abcdefghij"), 5, 0, SV("12345"), S("abcde12345fghij"));
    test(S("abcdefghij"), 5, 0, SV("1234567890"), S("abcde1234567890fghij"));
    test(S("abcdefghij"), 5, 0, SV("12345678901234567890"), S("abcde12345678901234567890fghij"));
    test(S("abcdefghij"), 5, 1, SV(""), S("abcdeghij"));
    test(S("abcdefghij"), 5, 1, SV("12345"), S("abcde12345ghij"));
    test(S("abcdefghij"), 5, 1, SV("1234567890"), S("abcde1234567890ghij"));
    test(S("abcdefghij"), 5, 1, SV("12345678901234567890"), S("abcde12345678901234567890ghij"));
    test(S("abcdefghij"), 5, 2, SV(""), S("abcdehij"));
    test(S("abcdefghij"), 5, 2, SV("12345"), S("abcde12345hij"));
    test(S("abcdefghij"), 5, 2, SV("1234567890"), S("abcde1234567890hij"));
    test(S("abcdefghij"), 5, 2, SV("12345678901234567890"), S("abcde12345678901234567890hij"));
    test(S("abcdefghij"), 5, 4, SV(""), S("abcdej"));
    test(S("abcdefghij"), 5, 4, SV("12345"), S("abcde12345j"));
    test(S("abcdefghij"), 5, 4, SV("1234567890"), S("abcde1234567890j"));
    test(S("abcdefghij"), 5, 4, SV("12345678901234567890"), S("abcde12345678901234567890j"));
    test(S("abcdefghij"), 5, 5, SV(""), S("abcde"));
    test(S("abcdefghij"), 5, 5, SV("12345"), S("abcde12345"));
    test(S("abcdefghij"), 5, 5, SV("1234567890"), S("abcde1234567890"));
    test(S("abcdefghij"), 5, 5, SV("12345678901234567890"), S("abcde12345678901234567890"));
    test(S("abcdefghij"), 9, 0, SV(""), S("abcdefghij"));
    test(S("abcdefghij"), 9, 0, SV("12345"), S("abcdefghi12345j"));
    test(S("abcdefghij"), 9, 0, SV("1234567890"), S("abcdefghi1234567890j"));
    test(S("abcdefghij"), 9, 0, SV("12345678901234567890"), S("abcdefghi12345678901234567890j"));
    test(S("abcdefghij"), 9, 1, SV(""), S("abcdefghi"));
    test(S("abcdefghij"), 9, 1, SV("12345"), S("abcdefghi12345"));
    test(S("abcdefghij"), 9, 1, SV("1234567890"), S("abcdefghi1234567890"));
    test(S("abcdefghij"), 9, 1, SV("12345678901234567890"), S("abcdefghi12345678901234567890"));
    test(S("abcdefghij"), 10, 0, SV(""), S("abcdefghij"));
    test(S("abcdefghij"), 10, 0, SV("12345"), S("abcdefghij12345"));
    test(S("abcdefghij"), 10, 0, SV("1234567890"), S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, 0, SV("12345678901234567890"), S("abcdefghij12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 0, 0, SV(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, SV("12345"), S("12345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, SV("1234567890"), S("1234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, SV("12345678901234567890"), S("12345678901234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, SV(""), S("bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, SV("12345"), S("12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, SV("1234567890"), S("1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, SV("12345678901234567890"), S("12345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, SV(""), S("klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, SV("12345"), S("12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, SV("1234567890"), S("1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, SV("12345678901234567890"), S("12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 19, SV(""), S("t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, SV("12345"), S("12345t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, SV("1234567890"), S("1234567890t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, SV("12345678901234567890"), S("12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 0, 20, SV(""), S(""));
    test(S("abcdefghijklmnopqrst"), 0, 20, SV("12345"), S("12345"));
    test(S("abcdefghijklmnopqrst"), 0, 20, SV("1234567890"), S("1234567890"));
    test(S("abcdefghijklmnopqrst"), 0, 20, SV("12345678901234567890"), S("12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 1, 0, SV(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, SV("12345"), S("a12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, SV("1234567890"), S("a1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, SV("12345678901234567890"), S("a12345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, SV(""), S("acdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, SV("12345"), S("a12345cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, SV("1234567890"), S("a1234567890cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, SV("12345678901234567890"), S("a12345678901234567890cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, SV(""), S("aklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, SV("12345"), S("a12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, SV("1234567890"), S("a1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, SV("12345678901234567890"), S("a12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 18, SV(""), S("at"));
    test(S("abcdefghijklmnopqrst"), 1, 18, SV("12345"), S("a12345t"));
    test(S("abcdefghijklmnopqrst"), 1, 18, SV("1234567890"), S("a1234567890t"));
    test(S("abcdefghijklmnopqrst"), 1, 18, SV("12345678901234567890"), S("a12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 1, 19, SV(""), S("a"));
    test(S("abcdefghijklmnopqrst"), 1, 19, SV("12345"), S("a12345"));
    test(S("abcdefghijklmnopqrst"), 1, 19, SV("1234567890"), S("a1234567890"));
    test(S("abcdefghijklmnopqrst"), 1, 19, SV("12345678901234567890"), S("a12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 10, 0, SV(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, SV("12345"), S("abcdefghij12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, SV("1234567890"), S("abcdefghij1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, SV("12345678901234567890"), S("abcdefghij12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, SV(""), S("abcdefghijlmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, SV("12345"), S("abcdefghij12345lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, SV("1234567890"), S("abcdefghij1234567890lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, SV("12345678901234567890"), S("abcdefghij12345678901234567890lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, SV(""), S("abcdefghijpqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, SV("12345"), S("abcdefghij12345pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, SV("1234567890"), S("abcdefghij1234567890pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, SV("12345678901234567890"), S("abcdefghij12345678901234567890pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 9, SV(""), S("abcdefghijt"));
    test(S("abcdefghijklmnopqrst"), 10, 9, SV("12345"), S("abcdefghij12345t"));
    test(S("abcdefghijklmnopqrst"), 10, 9, SV("1234567890"), S("abcdefghij1234567890t"));
    test(S("abcdefghijklmnopqrst"), 10, 9, SV("12345678901234567890"), S("abcdefghij12345678901234567890t"));
}

template <class S, class SV>
void test2()
{
    test(S("abcdefghijklmnopqrst"), 10, 10, SV(""), S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 10, 10, SV("12345"), S("abcdefghij12345"));
    test(S("abcdefghijklmnopqrst"), 10, 10, SV("1234567890"), S("abcdefghij1234567890"));
    test(S("abcdefghijklmnopqrst"), 10, 10, SV("12345678901234567890"), S("abcdefghij12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 19, 0, SV(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 0, SV("12345"), S("abcdefghijklmnopqrs12345t"));
    test(S("abcdefghijklmnopqrst"), 19, 0, SV("1234567890"), S("abcdefghijklmnopqrs1234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, 0, SV("12345678901234567890"), S("abcdefghijklmnopqrs12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, 1, SV(""), S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 19, 1, SV("12345"), S("abcdefghijklmnopqrs12345"));
    test(S("abcdefghijklmnopqrst"), 19, 1, SV("1234567890"), S("abcdefghijklmnopqrs1234567890"));
    test(S("abcdefghijklmnopqrst"), 19, 1, SV("12345678901234567890"), S("abcdefghijklmnopqrs12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 20, 0, SV(""), S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 0, SV("12345"), S("abcdefghijklmnopqrst12345"));
    test(S("abcdefghijklmnopqrst"), 20, 0, SV("1234567890"), S("abcdefghijklmnopqrst1234567890"));
    test(S("abcdefghijklmnopqrst"), 20, 0, SV("12345678901234567890"), S("abcdefghijklmnopqrst12345678901234567890"));
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
