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
//   replace(const_iterator i1, const_iterator i2, size_type n, charT c);

#include <string>
#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, typename S::size_type pos1, typename S::size_type n1, typename S::size_type n2,
     typename S::value_type c, S expected)
{
    typename S::size_type old_size = s.size();
    typename S::const_iterator first = s.begin() + pos1;
    typename S::const_iterator last = s.begin() + pos1 + n1;
    typename S::size_type xlen = last - first;
    s.replace(first, last, n2, c);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
    typename S::size_type rlen = n2;
    assert(s.size() == old_size - xlen + rlen);
}

template <class S>
void test0()
{
    test(S(""), 0, 0, 0, '3', S(""));
    test(S(""), 0, 0, 5, '3', S("33333"));
    test(S(""), 0, 0, 10, '3', S("3333333333"));
    test(S(""), 0, 0, 20, '3', S("33333333333333333333"));
    test(S("abcde"), 0, 0, 0, '3', S("abcde"));
    test(S("abcde"), 0, 0, 5, '3', S("33333abcde"));
    test(S("abcde"), 0, 0, 10, '3', S("3333333333abcde"));
    test(S("abcde"), 0, 0, 20, '3', S("33333333333333333333abcde"));
    test(S("abcde"), 0, 1, 0, '3', S("bcde"));
    test(S("abcde"), 0, 1, 5, '3', S("33333bcde"));
    test(S("abcde"), 0, 1, 10, '3', S("3333333333bcde"));
    test(S("abcde"), 0, 1, 20, '3', S("33333333333333333333bcde"));
    test(S("abcde"), 0, 2, 0, '3', S("cde"));
    test(S("abcde"), 0, 2, 5, '3', S("33333cde"));
    test(S("abcde"), 0, 2, 10, '3', S("3333333333cde"));
    test(S("abcde"), 0, 2, 20, '3', S("33333333333333333333cde"));
    test(S("abcde"), 0, 4, 0, '3', S("e"));
    test(S("abcde"), 0, 4, 5, '3', S("33333e"));
    test(S("abcde"), 0, 4, 10, '3', S("3333333333e"));
    test(S("abcde"), 0, 4, 20, '3', S("33333333333333333333e"));
    test(S("abcde"), 0, 5, 0, '3', S(""));
    test(S("abcde"), 0, 5, 5, '3', S("33333"));
    test(S("abcde"), 0, 5, 10, '3', S("3333333333"));
    test(S("abcde"), 0, 5, 20, '3', S("33333333333333333333"));
    test(S("abcde"), 1, 0, 0, '3', S("abcde"));
    test(S("abcde"), 1, 0, 5, '3', S("a33333bcde"));
    test(S("abcde"), 1, 0, 10, '3', S("a3333333333bcde"));
    test(S("abcde"), 1, 0, 20, '3', S("a33333333333333333333bcde"));
    test(S("abcde"), 1, 1, 0, '3', S("acde"));
    test(S("abcde"), 1, 1, 5, '3', S("a33333cde"));
    test(S("abcde"), 1, 1, 10, '3', S("a3333333333cde"));
    test(S("abcde"), 1, 1, 20, '3', S("a33333333333333333333cde"));
    test(S("abcde"), 1, 2, 0, '3', S("ade"));
    test(S("abcde"), 1, 2, 5, '3', S("a33333de"));
    test(S("abcde"), 1, 2, 10, '3', S("a3333333333de"));
    test(S("abcde"), 1, 2, 20, '3', S("a33333333333333333333de"));
    test(S("abcde"), 1, 3, 0, '3', S("ae"));
    test(S("abcde"), 1, 3, 5, '3', S("a33333e"));
    test(S("abcde"), 1, 3, 10, '3', S("a3333333333e"));
    test(S("abcde"), 1, 3, 20, '3', S("a33333333333333333333e"));
    test(S("abcde"), 1, 4, 0, '3', S("a"));
    test(S("abcde"), 1, 4, 5, '3', S("a33333"));
    test(S("abcde"), 1, 4, 10, '3', S("a3333333333"));
    test(S("abcde"), 1, 4, 20, '3', S("a33333333333333333333"));
    test(S("abcde"), 2, 0, 0, '3', S("abcde"));
    test(S("abcde"), 2, 0, 5, '3', S("ab33333cde"));
    test(S("abcde"), 2, 0, 10, '3', S("ab3333333333cde"));
    test(S("abcde"), 2, 0, 20, '3', S("ab33333333333333333333cde"));
    test(S("abcde"), 2, 1, 0, '3', S("abde"));
    test(S("abcde"), 2, 1, 5, '3', S("ab33333de"));
    test(S("abcde"), 2, 1, 10, '3', S("ab3333333333de"));
    test(S("abcde"), 2, 1, 20, '3', S("ab33333333333333333333de"));
    test(S("abcde"), 2, 2, 0, '3', S("abe"));
    test(S("abcde"), 2, 2, 5, '3', S("ab33333e"));
    test(S("abcde"), 2, 2, 10, '3', S("ab3333333333e"));
    test(S("abcde"), 2, 2, 20, '3', S("ab33333333333333333333e"));
    test(S("abcde"), 2, 3, 0, '3', S("ab"));
    test(S("abcde"), 2, 3, 5, '3', S("ab33333"));
    test(S("abcde"), 2, 3, 10, '3', S("ab3333333333"));
    test(S("abcde"), 2, 3, 20, '3', S("ab33333333333333333333"));
    test(S("abcde"), 4, 0, 0, '3', S("abcde"));
    test(S("abcde"), 4, 0, 5, '3', S("abcd33333e"));
    test(S("abcde"), 4, 0, 10, '3', S("abcd3333333333e"));
    test(S("abcde"), 4, 0, 20, '3', S("abcd33333333333333333333e"));
    test(S("abcde"), 4, 1, 0, '3', S("abcd"));
    test(S("abcde"), 4, 1, 5, '3', S("abcd33333"));
    test(S("abcde"), 4, 1, 10, '3', S("abcd3333333333"));
    test(S("abcde"), 4, 1, 20, '3', S("abcd33333333333333333333"));
    test(S("abcde"), 5, 0, 0, '3', S("abcde"));
    test(S("abcde"), 5, 0, 5, '3', S("abcde33333"));
    test(S("abcde"), 5, 0, 10, '3', S("abcde3333333333"));
    test(S("abcde"), 5, 0, 20, '3', S("abcde33333333333333333333"));
    test(S("abcdefghij"), 0, 0, 0, '3', S("abcdefghij"));
    test(S("abcdefghij"), 0, 0, 5, '3', S("33333abcdefghij"));
    test(S("abcdefghij"), 0, 0, 10, '3', S("3333333333abcdefghij"));
    test(S("abcdefghij"), 0, 0, 20, '3', S("33333333333333333333abcdefghij"));
    test(S("abcdefghij"), 0, 1, 0, '3', S("bcdefghij"));
    test(S("abcdefghij"), 0, 1, 5, '3', S("33333bcdefghij"));
    test(S("abcdefghij"), 0, 1, 10, '3', S("3333333333bcdefghij"));
    test(S("abcdefghij"), 0, 1, 20, '3', S("33333333333333333333bcdefghij"));
    test(S("abcdefghij"), 0, 5, 0, '3', S("fghij"));
    test(S("abcdefghij"), 0, 5, 5, '3', S("33333fghij"));
    test(S("abcdefghij"), 0, 5, 10, '3', S("3333333333fghij"));
    test(S("abcdefghij"), 0, 5, 20, '3', S("33333333333333333333fghij"));
    test(S("abcdefghij"), 0, 9, 0, '3', S("j"));
    test(S("abcdefghij"), 0, 9, 5, '3', S("33333j"));
    test(S("abcdefghij"), 0, 9, 10, '3', S("3333333333j"));
    test(S("abcdefghij"), 0, 9, 20, '3', S("33333333333333333333j"));
    test(S("abcdefghij"), 0, 10, 0, '3', S(""));
    test(S("abcdefghij"), 0, 10, 5, '3', S("33333"));
    test(S("abcdefghij"), 0, 10, 10, '3', S("3333333333"));
    test(S("abcdefghij"), 0, 10, 20, '3', S("33333333333333333333"));
    test(S("abcdefghij"), 1, 0, 0, '3', S("abcdefghij"));
    test(S("abcdefghij"), 1, 0, 5, '3', S("a33333bcdefghij"));
    test(S("abcdefghij"), 1, 0, 10, '3', S("a3333333333bcdefghij"));
    test(S("abcdefghij"), 1, 0, 20, '3', S("a33333333333333333333bcdefghij"));
    test(S("abcdefghij"), 1, 1, 0, '3', S("acdefghij"));
    test(S("abcdefghij"), 1, 1, 5, '3', S("a33333cdefghij"));
    test(S("abcdefghij"), 1, 1, 10, '3', S("a3333333333cdefghij"));
    test(S("abcdefghij"), 1, 1, 20, '3', S("a33333333333333333333cdefghij"));
}

template <class S>
void test1()
{
    test(S("abcdefghij"), 1, 4, 0, '3', S("afghij"));
    test(S("abcdefghij"), 1, 4, 5, '3', S("a33333fghij"));
    test(S("abcdefghij"), 1, 4, 10, '3', S("a3333333333fghij"));
    test(S("abcdefghij"), 1, 4, 20, '3', S("a33333333333333333333fghij"));
    test(S("abcdefghij"), 1, 8, 0, '3', S("aj"));
    test(S("abcdefghij"), 1, 8, 5, '3', S("a33333j"));
    test(S("abcdefghij"), 1, 8, 10, '3', S("a3333333333j"));
    test(S("abcdefghij"), 1, 8, 20, '3', S("a33333333333333333333j"));
    test(S("abcdefghij"), 1, 9, 0, '3', S("a"));
    test(S("abcdefghij"), 1, 9, 5, '3', S("a33333"));
    test(S("abcdefghij"), 1, 9, 10, '3', S("a3333333333"));
    test(S("abcdefghij"), 1, 9, 20, '3', S("a33333333333333333333"));
    test(S("abcdefghij"), 5, 0, 0, '3', S("abcdefghij"));
    test(S("abcdefghij"), 5, 0, 5, '3', S("abcde33333fghij"));
    test(S("abcdefghij"), 5, 0, 10, '3', S("abcde3333333333fghij"));
    test(S("abcdefghij"), 5, 0, 20, '3', S("abcde33333333333333333333fghij"));
    test(S("abcdefghij"), 5, 1, 0, '3', S("abcdeghij"));
    test(S("abcdefghij"), 5, 1, 5, '3', S("abcde33333ghij"));
    test(S("abcdefghij"), 5, 1, 10, '3', S("abcde3333333333ghij"));
    test(S("abcdefghij"), 5, 1, 20, '3', S("abcde33333333333333333333ghij"));
    test(S("abcdefghij"), 5, 2, 0, '3', S("abcdehij"));
    test(S("abcdefghij"), 5, 2, 5, '3', S("abcde33333hij"));
    test(S("abcdefghij"), 5, 2, 10, '3', S("abcde3333333333hij"));
    test(S("abcdefghij"), 5, 2, 20, '3', S("abcde33333333333333333333hij"));
    test(S("abcdefghij"), 5, 4, 0, '3', S("abcdej"));
    test(S("abcdefghij"), 5, 4, 5, '3', S("abcde33333j"));
    test(S("abcdefghij"), 5, 4, 10, '3', S("abcde3333333333j"));
    test(S("abcdefghij"), 5, 4, 20, '3', S("abcde33333333333333333333j"));
    test(S("abcdefghij"), 5, 5, 0, '3', S("abcde"));
    test(S("abcdefghij"), 5, 5, 5, '3', S("abcde33333"));
    test(S("abcdefghij"), 5, 5, 10, '3', S("abcde3333333333"));
    test(S("abcdefghij"), 5, 5, 20, '3', S("abcde33333333333333333333"));
    test(S("abcdefghij"), 9, 0, 0, '3', S("abcdefghij"));
    test(S("abcdefghij"), 9, 0, 5, '3', S("abcdefghi33333j"));
    test(S("abcdefghij"), 9, 0, 10, '3', S("abcdefghi3333333333j"));
    test(S("abcdefghij"), 9, 0, 20, '3', S("abcdefghi33333333333333333333j"));
    test(S("abcdefghij"), 9, 1, 0, '3', S("abcdefghi"));
    test(S("abcdefghij"), 9, 1, 5, '3', S("abcdefghi33333"));
    test(S("abcdefghij"), 9, 1, 10, '3', S("abcdefghi3333333333"));
    test(S("abcdefghij"), 9, 1, 20, '3', S("abcdefghi33333333333333333333"));
    test(S("abcdefghij"), 10, 0, 0, '3', S("abcdefghij"));
    test(S("abcdefghij"), 10, 0, 5, '3', S("abcdefghij33333"));
    test(S("abcdefghij"), 10, 0, 10, '3', S("abcdefghij3333333333"));
    test(S("abcdefghij"), 10, 0, 20, '3', S("abcdefghij33333333333333333333"));
    test(S("abcdefghijklmnopqrst"), 0, 0, 0, '3', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, 5, '3', S("33333abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, 10, '3', S("3333333333abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 0, 20, '3', S("33333333333333333333abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, 0, '3', S("bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, 5, '3', S("33333bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, 10, '3', S("3333333333bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 1, 20, '3', S("33333333333333333333bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, 0, '3', S("klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, 5, '3', S("33333klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, 10, '3', S("3333333333klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 10, 20, '3', S("33333333333333333333klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, 19, 0, '3', S("t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, 5, '3', S("33333t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, 10, '3', S("3333333333t"));
    test(S("abcdefghijklmnopqrst"), 0, 19, 20, '3', S("33333333333333333333t"));
    test(S("abcdefghijklmnopqrst"), 0, 20, 0, '3', S(""));
    test(S("abcdefghijklmnopqrst"), 0, 20, 5, '3', S("33333"));
    test(S("abcdefghijklmnopqrst"), 0, 20, 10, '3', S("3333333333"));
    test(S("abcdefghijklmnopqrst"), 0, 20, 20, '3', S("33333333333333333333"));
    test(S("abcdefghijklmnopqrst"), 1, 0, 0, '3', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, 5, '3', S("a33333bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, 10, '3', S("a3333333333bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 0, 20, '3', S("a33333333333333333333bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, 0, '3', S("acdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, 5, '3', S("a33333cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, 10, '3', S("a3333333333cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 1, 20, '3', S("a33333333333333333333cdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, 0, '3', S("aklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, 5, '3', S("a33333klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, 10, '3', S("a3333333333klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 9, 20, '3', S("a33333333333333333333klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, 18, 0, '3', S("at"));
    test(S("abcdefghijklmnopqrst"), 1, 18, 5, '3', S("a33333t"));
    test(S("abcdefghijklmnopqrst"), 1, 18, 10, '3', S("a3333333333t"));
    test(S("abcdefghijklmnopqrst"), 1, 18, 20, '3', S("a33333333333333333333t"));
    test(S("abcdefghijklmnopqrst"), 1, 19, 0, '3', S("a"));
    test(S("abcdefghijklmnopqrst"), 1, 19, 5, '3', S("a33333"));
    test(S("abcdefghijklmnopqrst"), 1, 19, 10, '3', S("a3333333333"));
    test(S("abcdefghijklmnopqrst"), 1, 19, 20, '3', S("a33333333333333333333"));
    test(S("abcdefghijklmnopqrst"), 10, 0, 0, '3', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, 5, '3', S("abcdefghij33333klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, 10, '3', S("abcdefghij3333333333klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 0, 20, '3', S("abcdefghij33333333333333333333klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, 0, '3', S("abcdefghijlmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, 5, '3', S("abcdefghij33333lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, 10, '3', S("abcdefghij3333333333lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 1, 20, '3', S("abcdefghij33333333333333333333lmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, 0, '3', S("abcdefghijpqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, 5, '3', S("abcdefghij33333pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, 10, '3', S("abcdefghij3333333333pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 5, 20, '3', S("abcdefghij33333333333333333333pqrst"));
    test(S("abcdefghijklmnopqrst"), 10, 9, 0, '3', S("abcdefghijt"));
    test(S("abcdefghijklmnopqrst"), 10, 9, 5, '3', S("abcdefghij33333t"));
    test(S("abcdefghijklmnopqrst"), 10, 9, 10, '3', S("abcdefghij3333333333t"));
    test(S("abcdefghijklmnopqrst"), 10, 9, 20, '3', S("abcdefghij33333333333333333333t"));
}

template <class S>
void test2()
{
    test(S("abcdefghijklmnopqrst"), 10, 10, 0, '3', S("abcdefghij"));
    test(S("abcdefghijklmnopqrst"), 10, 10, 5, '3', S("abcdefghij33333"));
    test(S("abcdefghijklmnopqrst"), 10, 10, 10, '3', S("abcdefghij3333333333"));
    test(S("abcdefghijklmnopqrst"), 10, 10, 20, '3', S("abcdefghij33333333333333333333"));
    test(S("abcdefghijklmnopqrst"), 19, 0, 0, '3', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, 0, 5, '3', S("abcdefghijklmnopqrs33333t"));
    test(S("abcdefghijklmnopqrst"), 19, 0, 10, '3', S("abcdefghijklmnopqrs3333333333t"));
    test(S("abcdefghijklmnopqrst"), 19, 0, 20, '3', S("abcdefghijklmnopqrs33333333333333333333t"));
    test(S("abcdefghijklmnopqrst"), 19, 1, 0, '3', S("abcdefghijklmnopqrs"));
    test(S("abcdefghijklmnopqrst"), 19, 1, 5, '3', S("abcdefghijklmnopqrs33333"));
    test(S("abcdefghijklmnopqrst"), 19, 1, 10, '3', S("abcdefghijklmnopqrs3333333333"));
    test(S("abcdefghijklmnopqrst"), 19, 1, 20, '3', S("abcdefghijklmnopqrs33333333333333333333"));
    test(S("abcdefghijklmnopqrst"), 20, 0, 0, '3', S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, 0, 5, '3', S("abcdefghijklmnopqrst33333"));
    test(S("abcdefghijklmnopqrst"), 20, 0, 10, '3', S("abcdefghijklmnopqrst3333333333"));
    test(S("abcdefghijklmnopqrst"), 20, 0, 20, '3', S("abcdefghijklmnopqrst33333333333333333333"));
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
