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
//   insert(size_type pos1, const basic_string<charT,traits,Allocator>& str,
//          size_type pos2, size_type n=npos);
// the "=npos" was added in C++14

#include <string>
#include <stdexcept>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, typename S::size_type pos1, S str, typename S::size_type pos2,
     typename S::size_type n, S expected)
{
    const typename S::size_type old_size = s.size();
    S s0 = s;
    if (pos1 <= old_size && pos2 <= str.size())
    {
        s.insert(pos1, str, pos2, n);
        LIBCPP_ASSERT(s.__invariants());
        assert(s == expected);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            s.insert(pos1, str, pos2, n);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos1 > old_size || pos2 > str.size());
            assert(s == s0);
        }
    }
#endif
}

template <class S>
void
test_npos(S s, typename S::size_type pos1, S str, typename S::size_type pos2, S expected)
{
    const typename S::size_type old_size = s.size();
    S s0 = s;
    if (pos1 <= old_size && pos2 <= str.size())
    {
        s.insert(pos1, str, pos2);
        LIBCPP_ASSERT(s.__invariants());
        assert(s == expected);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            s.insert(pos1, str, pos2);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos1 > old_size || pos2 > str.size());
            assert(s == s0);
        }
    }
#endif
}


template <class S>
void test0()
{
    test(S(""), 0, S(""), 0, 0, S(""));
    test(S(""), 0, S(""), 0, 1, S(""));
    test(S(""), 0, S(""), 1, 0, S("can't happen"));
    test(S(""), 0, S("12345"), 0, 0, S(""));
    test(S(""), 0, S("12345"), 0, 1, S("1"));
    test(S(""), 0, S("12345"), 0, 2, S("12"));
    test(S(""), 0, S("12345"), 0, 4, S("1234"));
    test(S(""), 0, S("12345"), 0, 5, S("12345"));
    test(S(""), 0, S("12345"), 0, 6, S("12345"));
    test(S(""), 0, S("12345"), 1, 0, S(""));
    test(S(""), 0, S("12345"), 1, 1, S("2"));
    test(S(""), 0, S("12345"), 1, 2, S("23"));
    test(S(""), 0, S("12345"), 1, 3, S("234"));
    test(S(""), 0, S("12345"), 1, 4, S("2345"));
    test(S(""), 0, S("12345"), 1, 5, S("2345"));
    test(S(""), 0, S("12345"), 2, 0, S(""));
    test(S(""), 0, S("12345"), 2, 1, S("3"));
    test(S(""), 0, S("12345"), 2, 2, S("34"));
    test(S(""), 0, S("12345"), 2, 3, S("345"));
    test(S(""), 0, S("12345"), 2, 4, S("345"));
    test(S(""), 0, S("12345"), 4, 0, S(""));
    test(S(""), 0, S("12345"), 4, 1, S("5"));
    test(S(""), 0, S("12345"), 4, 2, S("5"));
    test(S(""), 0, S("12345"), 5, 0, S(""));
    test(S(""), 0, S("12345"), 5, 1, S(""));
    test(S(""), 0, S("12345"), 6, 0, S("can't happen"));
    test(S(""), 0, S("1234567890"), 0, 0, S(""));
    test(S(""), 0, S("1234567890"), 0, 1, S("1"));
    test(S(""), 0, S("1234567890"), 0, 5, S("12345"));
    test(S(""), 0, S("1234567890"), 0, 9, S("123456789"));
    test(S(""), 0, S("1234567890"), 0, 10, S("1234567890"));
    test(S(""), 0, S("1234567890"), 0, 11, S("1234567890"));
    test(S(""), 0, S("1234567890"), 1, 0, S(""));
    test(S(""), 0, S("1234567890"), 1, 1, S("2"));
    test(S(""), 0, S("1234567890"), 1, 4, S("2345"));
    test(S(""), 0, S("1234567890"), 1, 8, S("23456789"));
    test(S(""), 0, S("1234567890"), 1, 9, S("234567890"));
    test(S(""), 0, S("1234567890"), 1, 10, S("234567890"));
    test(S(""), 0, S("1234567890"), 5, 0, S(""));
    test(S(""), 0, S("1234567890"), 5, 1, S("6"));
    test(S(""), 0, S("1234567890"), 5, 2, S("67"));
    test(S(""), 0, S("1234567890"), 5, 4, S("6789"));
    test(S(""), 0, S("1234567890"), 5, 5, S("67890"));
    test(S(""), 0, S("1234567890"), 5, 6, S("67890"));
    test(S(""), 0, S("1234567890"), 9, 0, S(""));
    test(S(""), 0, S("1234567890"), 9, 1, S("0"));
    test(S(""), 0, S("1234567890"), 9, 2, S("0"));
    test(S(""), 0, S("1234567890"), 10, 0, S(""));
    test(S(""), 0, S("1234567890"), 10, 1, S(""));
    test(S(""), 0, S("1234567890"), 11, 0, S("can't happen"));
}

template <class S>
void test1()
{
    test(S(""), 0, S("12345678901234567890"), 0, 0, S(""));
    test(S(""), 0, S("12345678901234567890"), 0, 1, S("1"));
    test(S(""), 0, S("12345678901234567890"), 0, 10, S("1234567890"));
    test(S(""), 0, S("12345678901234567890"), 0, 19, S("1234567890123456789"));
    test(S(""), 0, S("12345678901234567890"), 0, 20, S("12345678901234567890"));
    test(S(""), 0, S("12345678901234567890"), 0, 21, S("12345678901234567890"));
    test(S(""), 0, S("12345678901234567890"), 1, 0, S(""));
    test(S(""), 0, S("12345678901234567890"), 1, 1, S("2"));
    test(S(""), 0, S("12345678901234567890"), 1, 9, S("234567890"));
    test(S(""), 0, S("12345678901234567890"), 1, 18, S("234567890123456789"));
    test(S(""), 0, S("12345678901234567890"), 1, 19, S("2345678901234567890"));
    test(S(""), 0, S("12345678901234567890"), 1, 20, S("2345678901234567890"));
    test(S(""), 0, S("12345678901234567890"), 10, 0, S(""));
    test(S(""), 0, S("12345678901234567890"), 10, 1, S("1"));
    test(S(""), 0, S("12345678901234567890"), 10, 5, S("12345"));
    test(S(""), 0, S("12345678901234567890"), 10, 9, S("123456789"));
    test(S(""), 0, S("12345678901234567890"), 10, 10, S("1234567890"));
    test(S(""), 0, S("12345678901234567890"), 10, 11, S("1234567890"));
    test(S(""), 0, S("12345678901234567890"), 19, 0, S(""));
    test(S(""), 0, S("12345678901234567890"), 19, 1, S("0"));
    test(S(""), 0, S("12345678901234567890"), 19, 2, S("0"));
    test(S(""), 0, S("12345678901234567890"), 20, 0, S(""));
    test(S(""), 0, S("12345678901234567890"), 20, 1, S(""));
    test(S(""), 0, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S(""), 1, S(""), 0, 0, S("can't happen"));
    test(S(""), 1, S(""), 0, 1, S("can't happen"));
    test(S(""), 1, S(""), 1, 0, S("can't happen"));
    test(S(""), 1, S("12345"), 0, 0, S("can't happen"));
    test(S(""), 1, S("12345"), 0, 1, S("can't happen"));
    test(S(""), 1, S("12345"), 0, 2, S("can't happen"));
    test(S(""), 1, S("12345"), 0, 4, S("can't happen"));
    test(S(""), 1, S("12345"), 0, 5, S("can't happen"));
    test(S(""), 1, S("12345"), 0, 6, S("can't happen"));
    test(S(""), 1, S("12345"), 1, 0, S("can't happen"));
    test(S(""), 1, S("12345"), 1, 1, S("can't happen"));
    test(S(""), 1, S("12345"), 1, 2, S("can't happen"));
    test(S(""), 1, S("12345"), 1, 3, S("can't happen"));
    test(S(""), 1, S("12345"), 1, 4, S("can't happen"));
    test(S(""), 1, S("12345"), 1, 5, S("can't happen"));
    test(S(""), 1, S("12345"), 2, 0, S("can't happen"));
    test(S(""), 1, S("12345"), 2, 1, S("can't happen"));
    test(S(""), 1, S("12345"), 2, 2, S("can't happen"));
    test(S(""), 1, S("12345"), 2, 3, S("can't happen"));
    test(S(""), 1, S("12345"), 2, 4, S("can't happen"));
    test(S(""), 1, S("12345"), 4, 0, S("can't happen"));
    test(S(""), 1, S("12345"), 4, 1, S("can't happen"));
    test(S(""), 1, S("12345"), 4, 2, S("can't happen"));
    test(S(""), 1, S("12345"), 5, 0, S("can't happen"));
    test(S(""), 1, S("12345"), 5, 1, S("can't happen"));
    test(S(""), 1, S("12345"), 6, 0, S("can't happen"));
}

template <class S>
void test2()
{
    test(S(""), 1, S("1234567890"), 0, 0, S("can't happen"));
    test(S(""), 1, S("1234567890"), 0, 1, S("can't happen"));
    test(S(""), 1, S("1234567890"), 0, 5, S("can't happen"));
    test(S(""), 1, S("1234567890"), 0, 9, S("can't happen"));
    test(S(""), 1, S("1234567890"), 0, 10, S("can't happen"));
    test(S(""), 1, S("1234567890"), 0, 11, S("can't happen"));
    test(S(""), 1, S("1234567890"), 1, 0, S("can't happen"));
    test(S(""), 1, S("1234567890"), 1, 1, S("can't happen"));
    test(S(""), 1, S("1234567890"), 1, 4, S("can't happen"));
    test(S(""), 1, S("1234567890"), 1, 8, S("can't happen"));
    test(S(""), 1, S("1234567890"), 1, 9, S("can't happen"));
    test(S(""), 1, S("1234567890"), 1, 10, S("can't happen"));
    test(S(""), 1, S("1234567890"), 5, 0, S("can't happen"));
    test(S(""), 1, S("1234567890"), 5, 1, S("can't happen"));
    test(S(""), 1, S("1234567890"), 5, 2, S("can't happen"));
    test(S(""), 1, S("1234567890"), 5, 4, S("can't happen"));
    test(S(""), 1, S("1234567890"), 5, 5, S("can't happen"));
    test(S(""), 1, S("1234567890"), 5, 6, S("can't happen"));
    test(S(""), 1, S("1234567890"), 9, 0, S("can't happen"));
    test(S(""), 1, S("1234567890"), 9, 1, S("can't happen"));
    test(S(""), 1, S("1234567890"), 9, 2, S("can't happen"));
    test(S(""), 1, S("1234567890"), 10, 0, S("can't happen"));
    test(S(""), 1, S("1234567890"), 10, 1, S("can't happen"));
    test(S(""), 1, S("1234567890"), 11, 0, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 0, 0, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 0, 1, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 0, 10, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 0, 19, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 0, 20, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 0, 21, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 1, 0, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 1, 1, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 1, 9, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 1, 18, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 1, 19, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 1, 20, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 10, 0, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 10, 1, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 10, 5, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 10, 9, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 10, 10, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 10, 11, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 19, 0, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 19, 1, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 19, 2, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 20, 0, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 20, 1, S("can't happen"));
    test(S(""), 1, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcde"), 0, S(""), 0, 0, S("abcde"));
    test(S("abcde"), 0, S(""), 0, 1, S("abcde"));
}

template <class S>
void test3()
{
    test(S("abcde"), 0, S(""), 1, 0, S("can't happen"));
    test(S("abcde"), 0, S("12345"), 0, 0, S("abcde"));
    test(S("abcde"), 0, S("12345"), 0, 1, S("1abcde"));
    test(S("abcde"), 0, S("12345"), 0, 2, S("12abcde"));
    test(S("abcde"), 0, S("12345"), 0, 4, S("1234abcde"));
    test(S("abcde"), 0, S("12345"), 0, 5, S("12345abcde"));
    test(S("abcde"), 0, S("12345"), 0, 6, S("12345abcde"));
    test(S("abcde"), 0, S("12345"), 1, 0, S("abcde"));
    test(S("abcde"), 0, S("12345"), 1, 1, S("2abcde"));
    test(S("abcde"), 0, S("12345"), 1, 2, S("23abcde"));
    test(S("abcde"), 0, S("12345"), 1, 3, S("234abcde"));
    test(S("abcde"), 0, S("12345"), 1, 4, S("2345abcde"));
    test(S("abcde"), 0, S("12345"), 1, 5, S("2345abcde"));
    test(S("abcde"), 0, S("12345"), 2, 0, S("abcde"));
    test(S("abcde"), 0, S("12345"), 2, 1, S("3abcde"));
    test(S("abcde"), 0, S("12345"), 2, 2, S("34abcde"));
    test(S("abcde"), 0, S("12345"), 2, 3, S("345abcde"));
    test(S("abcde"), 0, S("12345"), 2, 4, S("345abcde"));
    test(S("abcde"), 0, S("12345"), 4, 0, S("abcde"));
    test(S("abcde"), 0, S("12345"), 4, 1, S("5abcde"));
    test(S("abcde"), 0, S("12345"), 4, 2, S("5abcde"));
    test(S("abcde"), 0, S("12345"), 5, 0, S("abcde"));
    test(S("abcde"), 0, S("12345"), 5, 1, S("abcde"));
    test(S("abcde"), 0, S("12345"), 6, 0, S("can't happen"));
    test(S("abcde"), 0, S("1234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 0, S("1234567890"), 0, 1, S("1abcde"));
    test(S("abcde"), 0, S("1234567890"), 0, 5, S("12345abcde"));
    test(S("abcde"), 0, S("1234567890"), 0, 9, S("123456789abcde"));
    test(S("abcde"), 0, S("1234567890"), 0, 10, S("1234567890abcde"));
    test(S("abcde"), 0, S("1234567890"), 0, 11, S("1234567890abcde"));
    test(S("abcde"), 0, S("1234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 0, S("1234567890"), 1, 1, S("2abcde"));
    test(S("abcde"), 0, S("1234567890"), 1, 4, S("2345abcde"));
    test(S("abcde"), 0, S("1234567890"), 1, 8, S("23456789abcde"));
    test(S("abcde"), 0, S("1234567890"), 1, 9, S("234567890abcde"));
    test(S("abcde"), 0, S("1234567890"), 1, 10, S("234567890abcde"));
    test(S("abcde"), 0, S("1234567890"), 5, 0, S("abcde"));
    test(S("abcde"), 0, S("1234567890"), 5, 1, S("6abcde"));
    test(S("abcde"), 0, S("1234567890"), 5, 2, S("67abcde"));
    test(S("abcde"), 0, S("1234567890"), 5, 4, S("6789abcde"));
    test(S("abcde"), 0, S("1234567890"), 5, 5, S("67890abcde"));
    test(S("abcde"), 0, S("1234567890"), 5, 6, S("67890abcde"));
    test(S("abcde"), 0, S("1234567890"), 9, 0, S("abcde"));
    test(S("abcde"), 0, S("1234567890"), 9, 1, S("0abcde"));
    test(S("abcde"), 0, S("1234567890"), 9, 2, S("0abcde"));
    test(S("abcde"), 0, S("1234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 0, S("1234567890"), 10, 1, S("abcde"));
    test(S("abcde"), 0, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcde"), 0, S("12345678901234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 0, 1, S("1abcde"));
}

template <class S>
void test4()
{
    test(S("abcde"), 0, S("12345678901234567890"), 0, 10, S("1234567890abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 0, 19, S("1234567890123456789abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 0, 20, S("12345678901234567890abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 0, 21, S("12345678901234567890abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 1, 1, S("2abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 1, 9, S("234567890abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 1, 18, S("234567890123456789abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 1, 19, S("2345678901234567890abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 1, 20, S("2345678901234567890abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 10, 1, S("1abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 10, 5, S("12345abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 10, 9, S("123456789abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 10, 10, S("1234567890abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 10, 11, S("1234567890abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 19, 0, S("abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 19, 1, S("0abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 19, 2, S("0abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 20, 0, S("abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 20, 1, S("abcde"));
    test(S("abcde"), 0, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcde"), 1, S(""), 0, 0, S("abcde"));
    test(S("abcde"), 1, S(""), 0, 1, S("abcde"));
    test(S("abcde"), 1, S(""), 1, 0, S("can't happen"));
    test(S("abcde"), 1, S("12345"), 0, 0, S("abcde"));
    test(S("abcde"), 1, S("12345"), 0, 1, S("a1bcde"));
    test(S("abcde"), 1, S("12345"), 0, 2, S("a12bcde"));
    test(S("abcde"), 1, S("12345"), 0, 4, S("a1234bcde"));
    test(S("abcde"), 1, S("12345"), 0, 5, S("a12345bcde"));
    test(S("abcde"), 1, S("12345"), 0, 6, S("a12345bcde"));
    test(S("abcde"), 1, S("12345"), 1, 0, S("abcde"));
    test(S("abcde"), 1, S("12345"), 1, 1, S("a2bcde"));
    test(S("abcde"), 1, S("12345"), 1, 2, S("a23bcde"));
    test(S("abcde"), 1, S("12345"), 1, 3, S("a234bcde"));
    test(S("abcde"), 1, S("12345"), 1, 4, S("a2345bcde"));
    test(S("abcde"), 1, S("12345"), 1, 5, S("a2345bcde"));
    test(S("abcde"), 1, S("12345"), 2, 0, S("abcde"));
    test(S("abcde"), 1, S("12345"), 2, 1, S("a3bcde"));
    test(S("abcde"), 1, S("12345"), 2, 2, S("a34bcde"));
    test(S("abcde"), 1, S("12345"), 2, 3, S("a345bcde"));
    test(S("abcde"), 1, S("12345"), 2, 4, S("a345bcde"));
    test(S("abcde"), 1, S("12345"), 4, 0, S("abcde"));
    test(S("abcde"), 1, S("12345"), 4, 1, S("a5bcde"));
    test(S("abcde"), 1, S("12345"), 4, 2, S("a5bcde"));
    test(S("abcde"), 1, S("12345"), 5, 0, S("abcde"));
    test(S("abcde"), 1, S("12345"), 5, 1, S("abcde"));
    test(S("abcde"), 1, S("12345"), 6, 0, S("can't happen"));
    test(S("abcde"), 1, S("1234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 1, S("1234567890"), 0, 1, S("a1bcde"));
}

template <class S>
void test5()
{
    test(S("abcde"), 1, S("1234567890"), 0, 5, S("a12345bcde"));
    test(S("abcde"), 1, S("1234567890"), 0, 9, S("a123456789bcde"));
    test(S("abcde"), 1, S("1234567890"), 0, 10, S("a1234567890bcde"));
    test(S("abcde"), 1, S("1234567890"), 0, 11, S("a1234567890bcde"));
    test(S("abcde"), 1, S("1234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 1, S("1234567890"), 1, 1, S("a2bcde"));
    test(S("abcde"), 1, S("1234567890"), 1, 4, S("a2345bcde"));
    test(S("abcde"), 1, S("1234567890"), 1, 8, S("a23456789bcde"));
    test(S("abcde"), 1, S("1234567890"), 1, 9, S("a234567890bcde"));
    test(S("abcde"), 1, S("1234567890"), 1, 10, S("a234567890bcde"));
    test(S("abcde"), 1, S("1234567890"), 5, 0, S("abcde"));
    test(S("abcde"), 1, S("1234567890"), 5, 1, S("a6bcde"));
    test(S("abcde"), 1, S("1234567890"), 5, 2, S("a67bcde"));
    test(S("abcde"), 1, S("1234567890"), 5, 4, S("a6789bcde"));
    test(S("abcde"), 1, S("1234567890"), 5, 5, S("a67890bcde"));
    test(S("abcde"), 1, S("1234567890"), 5, 6, S("a67890bcde"));
    test(S("abcde"), 1, S("1234567890"), 9, 0, S("abcde"));
    test(S("abcde"), 1, S("1234567890"), 9, 1, S("a0bcde"));
    test(S("abcde"), 1, S("1234567890"), 9, 2, S("a0bcde"));
    test(S("abcde"), 1, S("1234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 1, S("1234567890"), 10, 1, S("abcde"));
    test(S("abcde"), 1, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcde"), 1, S("12345678901234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 0, 1, S("a1bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 0, 10, S("a1234567890bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 0, 19, S("a1234567890123456789bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 0, 20, S("a12345678901234567890bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 0, 21, S("a12345678901234567890bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 1, 1, S("a2bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 1, 9, S("a234567890bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 1, 18, S("a234567890123456789bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 1, 19, S("a2345678901234567890bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 1, 20, S("a2345678901234567890bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 10, 1, S("a1bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 10, 5, S("a12345bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 10, 9, S("a123456789bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 10, 10, S("a1234567890bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 10, 11, S("a1234567890bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 19, 0, S("abcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 19, 1, S("a0bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 19, 2, S("a0bcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 20, 0, S("abcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 20, 1, S("abcde"));
    test(S("abcde"), 1, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcde"), 2, S(""), 0, 0, S("abcde"));
    test(S("abcde"), 2, S(""), 0, 1, S("abcde"));
    test(S("abcde"), 2, S(""), 1, 0, S("can't happen"));
    test(S("abcde"), 2, S("12345"), 0, 0, S("abcde"));
}

template <class S>
void test6()
{
    test(S("abcde"), 2, S("12345"), 0, 1, S("ab1cde"));
    test(S("abcde"), 2, S("12345"), 0, 2, S("ab12cde"));
    test(S("abcde"), 2, S("12345"), 0, 4, S("ab1234cde"));
    test(S("abcde"), 2, S("12345"), 0, 5, S("ab12345cde"));
    test(S("abcde"), 2, S("12345"), 0, 6, S("ab12345cde"));
    test(S("abcde"), 2, S("12345"), 1, 0, S("abcde"));
    test(S("abcde"), 2, S("12345"), 1, 1, S("ab2cde"));
    test(S("abcde"), 2, S("12345"), 1, 2, S("ab23cde"));
    test(S("abcde"), 2, S("12345"), 1, 3, S("ab234cde"));
    test(S("abcde"), 2, S("12345"), 1, 4, S("ab2345cde"));
    test(S("abcde"), 2, S("12345"), 1, 5, S("ab2345cde"));
    test(S("abcde"), 2, S("12345"), 2, 0, S("abcde"));
    test(S("abcde"), 2, S("12345"), 2, 1, S("ab3cde"));
    test(S("abcde"), 2, S("12345"), 2, 2, S("ab34cde"));
    test(S("abcde"), 2, S("12345"), 2, 3, S("ab345cde"));
    test(S("abcde"), 2, S("12345"), 2, 4, S("ab345cde"));
    test(S("abcde"), 2, S("12345"), 4, 0, S("abcde"));
    test(S("abcde"), 2, S("12345"), 4, 1, S("ab5cde"));
    test(S("abcde"), 2, S("12345"), 4, 2, S("ab5cde"));
    test(S("abcde"), 2, S("12345"), 5, 0, S("abcde"));
    test(S("abcde"), 2, S("12345"), 5, 1, S("abcde"));
    test(S("abcde"), 2, S("12345"), 6, 0, S("can't happen"));
    test(S("abcde"), 2, S("1234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 2, S("1234567890"), 0, 1, S("ab1cde"));
    test(S("abcde"), 2, S("1234567890"), 0, 5, S("ab12345cde"));
    test(S("abcde"), 2, S("1234567890"), 0, 9, S("ab123456789cde"));
    test(S("abcde"), 2, S("1234567890"), 0, 10, S("ab1234567890cde"));
    test(S("abcde"), 2, S("1234567890"), 0, 11, S("ab1234567890cde"));
    test(S("abcde"), 2, S("1234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 2, S("1234567890"), 1, 1, S("ab2cde"));
    test(S("abcde"), 2, S("1234567890"), 1, 4, S("ab2345cde"));
    test(S("abcde"), 2, S("1234567890"), 1, 8, S("ab23456789cde"));
    test(S("abcde"), 2, S("1234567890"), 1, 9, S("ab234567890cde"));
    test(S("abcde"), 2, S("1234567890"), 1, 10, S("ab234567890cde"));
    test(S("abcde"), 2, S("1234567890"), 5, 0, S("abcde"));
    test(S("abcde"), 2, S("1234567890"), 5, 1, S("ab6cde"));
    test(S("abcde"), 2, S("1234567890"), 5, 2, S("ab67cde"));
    test(S("abcde"), 2, S("1234567890"), 5, 4, S("ab6789cde"));
    test(S("abcde"), 2, S("1234567890"), 5, 5, S("ab67890cde"));
    test(S("abcde"), 2, S("1234567890"), 5, 6, S("ab67890cde"));
    test(S("abcde"), 2, S("1234567890"), 9, 0, S("abcde"));
    test(S("abcde"), 2, S("1234567890"), 9, 1, S("ab0cde"));
    test(S("abcde"), 2, S("1234567890"), 9, 2, S("ab0cde"));
    test(S("abcde"), 2, S("1234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 2, S("1234567890"), 10, 1, S("abcde"));
    test(S("abcde"), 2, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcde"), 2, S("12345678901234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 2, S("12345678901234567890"), 0, 1, S("ab1cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 0, 10, S("ab1234567890cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 0, 19, S("ab1234567890123456789cde"));
}

template <class S>
void test7()
{
    test(S("abcde"), 2, S("12345678901234567890"), 0, 20, S("ab12345678901234567890cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 0, 21, S("ab12345678901234567890cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 2, S("12345678901234567890"), 1, 1, S("ab2cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 1, 9, S("ab234567890cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 1, 18, S("ab234567890123456789cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 1, 19, S("ab2345678901234567890cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 1, 20, S("ab2345678901234567890cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 2, S("12345678901234567890"), 10, 1, S("ab1cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 10, 5, S("ab12345cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 10, 9, S("ab123456789cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 10, 10, S("ab1234567890cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 10, 11, S("ab1234567890cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 19, 0, S("abcde"));
    test(S("abcde"), 2, S("12345678901234567890"), 19, 1, S("ab0cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 19, 2, S("ab0cde"));
    test(S("abcde"), 2, S("12345678901234567890"), 20, 0, S("abcde"));
    test(S("abcde"), 2, S("12345678901234567890"), 20, 1, S("abcde"));
    test(S("abcde"), 2, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcde"), 4, S(""), 0, 0, S("abcde"));
    test(S("abcde"), 4, S(""), 0, 1, S("abcde"));
    test(S("abcde"), 4, S(""), 1, 0, S("can't happen"));
    test(S("abcde"), 4, S("12345"), 0, 0, S("abcde"));
    test(S("abcde"), 4, S("12345"), 0, 1, S("abcd1e"));
    test(S("abcde"), 4, S("12345"), 0, 2, S("abcd12e"));
    test(S("abcde"), 4, S("12345"), 0, 4, S("abcd1234e"));
    test(S("abcde"), 4, S("12345"), 0, 5, S("abcd12345e"));
    test(S("abcde"), 4, S("12345"), 0, 6, S("abcd12345e"));
    test(S("abcde"), 4, S("12345"), 1, 0, S("abcde"));
    test(S("abcde"), 4, S("12345"), 1, 1, S("abcd2e"));
    test(S("abcde"), 4, S("12345"), 1, 2, S("abcd23e"));
    test(S("abcde"), 4, S("12345"), 1, 3, S("abcd234e"));
    test(S("abcde"), 4, S("12345"), 1, 4, S("abcd2345e"));
    test(S("abcde"), 4, S("12345"), 1, 5, S("abcd2345e"));
    test(S("abcde"), 4, S("12345"), 2, 0, S("abcde"));
    test(S("abcde"), 4, S("12345"), 2, 1, S("abcd3e"));
    test(S("abcde"), 4, S("12345"), 2, 2, S("abcd34e"));
    test(S("abcde"), 4, S("12345"), 2, 3, S("abcd345e"));
    test(S("abcde"), 4, S("12345"), 2, 4, S("abcd345e"));
    test(S("abcde"), 4, S("12345"), 4, 0, S("abcde"));
    test(S("abcde"), 4, S("12345"), 4, 1, S("abcd5e"));
    test(S("abcde"), 4, S("12345"), 4, 2, S("abcd5e"));
    test(S("abcde"), 4, S("12345"), 5, 0, S("abcde"));
    test(S("abcde"), 4, S("12345"), 5, 1, S("abcde"));
    test(S("abcde"), 4, S("12345"), 6, 0, S("can't happen"));
    test(S("abcde"), 4, S("1234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 4, S("1234567890"), 0, 1, S("abcd1e"));
    test(S("abcde"), 4, S("1234567890"), 0, 5, S("abcd12345e"));
    test(S("abcde"), 4, S("1234567890"), 0, 9, S("abcd123456789e"));
}

template <class S>
void test8()
{
    test(S("abcde"), 4, S("1234567890"), 0, 10, S("abcd1234567890e"));
    test(S("abcde"), 4, S("1234567890"), 0, 11, S("abcd1234567890e"));
    test(S("abcde"), 4, S("1234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 4, S("1234567890"), 1, 1, S("abcd2e"));
    test(S("abcde"), 4, S("1234567890"), 1, 4, S("abcd2345e"));
    test(S("abcde"), 4, S("1234567890"), 1, 8, S("abcd23456789e"));
    test(S("abcde"), 4, S("1234567890"), 1, 9, S("abcd234567890e"));
    test(S("abcde"), 4, S("1234567890"), 1, 10, S("abcd234567890e"));
    test(S("abcde"), 4, S("1234567890"), 5, 0, S("abcde"));
    test(S("abcde"), 4, S("1234567890"), 5, 1, S("abcd6e"));
    test(S("abcde"), 4, S("1234567890"), 5, 2, S("abcd67e"));
    test(S("abcde"), 4, S("1234567890"), 5, 4, S("abcd6789e"));
    test(S("abcde"), 4, S("1234567890"), 5, 5, S("abcd67890e"));
    test(S("abcde"), 4, S("1234567890"), 5, 6, S("abcd67890e"));
    test(S("abcde"), 4, S("1234567890"), 9, 0, S("abcde"));
    test(S("abcde"), 4, S("1234567890"), 9, 1, S("abcd0e"));
    test(S("abcde"), 4, S("1234567890"), 9, 2, S("abcd0e"));
    test(S("abcde"), 4, S("1234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 4, S("1234567890"), 10, 1, S("abcde"));
    test(S("abcde"), 4, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcde"), 4, S("12345678901234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 4, S("12345678901234567890"), 0, 1, S("abcd1e"));
    test(S("abcde"), 4, S("12345678901234567890"), 0, 10, S("abcd1234567890e"));
    test(S("abcde"), 4, S("12345678901234567890"), 0, 19, S("abcd1234567890123456789e"));
    test(S("abcde"), 4, S("12345678901234567890"), 0, 20, S("abcd12345678901234567890e"));
    test(S("abcde"), 4, S("12345678901234567890"), 0, 21, S("abcd12345678901234567890e"));
    test(S("abcde"), 4, S("12345678901234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 4, S("12345678901234567890"), 1, 1, S("abcd2e"));
    test(S("abcde"), 4, S("12345678901234567890"), 1, 9, S("abcd234567890e"));
    test(S("abcde"), 4, S("12345678901234567890"), 1, 18, S("abcd234567890123456789e"));
    test(S("abcde"), 4, S("12345678901234567890"), 1, 19, S("abcd2345678901234567890e"));
    test(S("abcde"), 4, S("12345678901234567890"), 1, 20, S("abcd2345678901234567890e"));
    test(S("abcde"), 4, S("12345678901234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 4, S("12345678901234567890"), 10, 1, S("abcd1e"));
    test(S("abcde"), 4, S("12345678901234567890"), 10, 5, S("abcd12345e"));
    test(S("abcde"), 4, S("12345678901234567890"), 10, 9, S("abcd123456789e"));
    test(S("abcde"), 4, S("12345678901234567890"), 10, 10, S("abcd1234567890e"));
    test(S("abcde"), 4, S("12345678901234567890"), 10, 11, S("abcd1234567890e"));
    test(S("abcde"), 4, S("12345678901234567890"), 19, 0, S("abcde"));
    test(S("abcde"), 4, S("12345678901234567890"), 19, 1, S("abcd0e"));
    test(S("abcde"), 4, S("12345678901234567890"), 19, 2, S("abcd0e"));
    test(S("abcde"), 4, S("12345678901234567890"), 20, 0, S("abcde"));
    test(S("abcde"), 4, S("12345678901234567890"), 20, 1, S("abcde"));
    test(S("abcde"), 4, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcde"), 5, S(""), 0, 0, S("abcde"));
    test(S("abcde"), 5, S(""), 0, 1, S("abcde"));
    test(S("abcde"), 5, S(""), 1, 0, S("can't happen"));
    test(S("abcde"), 5, S("12345"), 0, 0, S("abcde"));
    test(S("abcde"), 5, S("12345"), 0, 1, S("abcde1"));
    test(S("abcde"), 5, S("12345"), 0, 2, S("abcde12"));
}

template <class S>
void test9()
{
    test(S("abcde"), 5, S("12345"), 0, 4, S("abcde1234"));
    test(S("abcde"), 5, S("12345"), 0, 5, S("abcde12345"));
    test(S("abcde"), 5, S("12345"), 0, 6, S("abcde12345"));
    test(S("abcde"), 5, S("12345"), 1, 0, S("abcde"));
    test(S("abcde"), 5, S("12345"), 1, 1, S("abcde2"));
    test(S("abcde"), 5, S("12345"), 1, 2, S("abcde23"));
    test(S("abcde"), 5, S("12345"), 1, 3, S("abcde234"));
    test(S("abcde"), 5, S("12345"), 1, 4, S("abcde2345"));
    test(S("abcde"), 5, S("12345"), 1, 5, S("abcde2345"));
    test(S("abcde"), 5, S("12345"), 2, 0, S("abcde"));
    test(S("abcde"), 5, S("12345"), 2, 1, S("abcde3"));
    test(S("abcde"), 5, S("12345"), 2, 2, S("abcde34"));
    test(S("abcde"), 5, S("12345"), 2, 3, S("abcde345"));
    test(S("abcde"), 5, S("12345"), 2, 4, S("abcde345"));
    test(S("abcde"), 5, S("12345"), 4, 0, S("abcde"));
    test(S("abcde"), 5, S("12345"), 4, 1, S("abcde5"));
    test(S("abcde"), 5, S("12345"), 4, 2, S("abcde5"));
    test(S("abcde"), 5, S("12345"), 5, 0, S("abcde"));
    test(S("abcde"), 5, S("12345"), 5, 1, S("abcde"));
    test(S("abcde"), 5, S("12345"), 6, 0, S("can't happen"));
    test(S("abcde"), 5, S("1234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 5, S("1234567890"), 0, 1, S("abcde1"));
    test(S("abcde"), 5, S("1234567890"), 0, 5, S("abcde12345"));
    test(S("abcde"), 5, S("1234567890"), 0, 9, S("abcde123456789"));
    test(S("abcde"), 5, S("1234567890"), 0, 10, S("abcde1234567890"));
    test(S("abcde"), 5, S("1234567890"), 0, 11, S("abcde1234567890"));
    test(S("abcde"), 5, S("1234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 5, S("1234567890"), 1, 1, S("abcde2"));
    test(S("abcde"), 5, S("1234567890"), 1, 4, S("abcde2345"));
    test(S("abcde"), 5, S("1234567890"), 1, 8, S("abcde23456789"));
    test(S("abcde"), 5, S("1234567890"), 1, 9, S("abcde234567890"));
    test(S("abcde"), 5, S("1234567890"), 1, 10, S("abcde234567890"));
    test(S("abcde"), 5, S("1234567890"), 5, 0, S("abcde"));
    test(S("abcde"), 5, S("1234567890"), 5, 1, S("abcde6"));
    test(S("abcde"), 5, S("1234567890"), 5, 2, S("abcde67"));
    test(S("abcde"), 5, S("1234567890"), 5, 4, S("abcde6789"));
    test(S("abcde"), 5, S("1234567890"), 5, 5, S("abcde67890"));
    test(S("abcde"), 5, S("1234567890"), 5, 6, S("abcde67890"));
    test(S("abcde"), 5, S("1234567890"), 9, 0, S("abcde"));
    test(S("abcde"), 5, S("1234567890"), 9, 1, S("abcde0"));
    test(S("abcde"), 5, S("1234567890"), 9, 2, S("abcde0"));
    test(S("abcde"), 5, S("1234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 5, S("1234567890"), 10, 1, S("abcde"));
    test(S("abcde"), 5, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcde"), 5, S("12345678901234567890"), 0, 0, S("abcde"));
    test(S("abcde"), 5, S("12345678901234567890"), 0, 1, S("abcde1"));
    test(S("abcde"), 5, S("12345678901234567890"), 0, 10, S("abcde1234567890"));
    test(S("abcde"), 5, S("12345678901234567890"), 0, 19, S("abcde1234567890123456789"));
    test(S("abcde"), 5, S("12345678901234567890"), 0, 20, S("abcde12345678901234567890"));
    test(S("abcde"), 5, S("12345678901234567890"), 0, 21, S("abcde12345678901234567890"));
}

template <class S>
void test10()
{
    test(S("abcde"), 5, S("12345678901234567890"), 1, 0, S("abcde"));
    test(S("abcde"), 5, S("12345678901234567890"), 1, 1, S("abcde2"));
    test(S("abcde"), 5, S("12345678901234567890"), 1, 9, S("abcde234567890"));
    test(S("abcde"), 5, S("12345678901234567890"), 1, 18, S("abcde234567890123456789"));
    test(S("abcde"), 5, S("12345678901234567890"), 1, 19, S("abcde2345678901234567890"));
    test(S("abcde"), 5, S("12345678901234567890"), 1, 20, S("abcde2345678901234567890"));
    test(S("abcde"), 5, S("12345678901234567890"), 10, 0, S("abcde"));
    test(S("abcde"), 5, S("12345678901234567890"), 10, 1, S("abcde1"));
    test(S("abcde"), 5, S("12345678901234567890"), 10, 5, S("abcde12345"));
    test(S("abcde"), 5, S("12345678901234567890"), 10, 9, S("abcde123456789"));
    test(S("abcde"), 5, S("12345678901234567890"), 10, 10, S("abcde1234567890"));
    test(S("abcde"), 5, S("12345678901234567890"), 10, 11, S("abcde1234567890"));
    test(S("abcde"), 5, S("12345678901234567890"), 19, 0, S("abcde"));
    test(S("abcde"), 5, S("12345678901234567890"), 19, 1, S("abcde0"));
    test(S("abcde"), 5, S("12345678901234567890"), 19, 2, S("abcde0"));
    test(S("abcde"), 5, S("12345678901234567890"), 20, 0, S("abcde"));
    test(S("abcde"), 5, S("12345678901234567890"), 20, 1, S("abcde"));
    test(S("abcde"), 5, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcde"), 6, S(""), 0, 0, S("can't happen"));
    test(S("abcde"), 6, S(""), 0, 1, S("can't happen"));
    test(S("abcde"), 6, S(""), 1, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 0, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 0, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 0, 2, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 0, 4, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 0, 5, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 0, 6, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 1, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 1, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 1, 2, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 1, 3, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 1, 4, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 1, 5, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 2, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 2, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 2, 2, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 2, 3, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 2, 4, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 4, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 4, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 4, 2, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 5, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 5, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345"), 6, 0, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 0, 0, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 0, 1, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 0, 5, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 0, 9, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 0, 10, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 0, 11, S("can't happen"));
}

template <class S>
void test11()
{
    test(S("abcde"), 6, S("1234567890"), 1, 0, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 1, 1, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 1, 4, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 1, 8, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 1, 9, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 1, 10, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 5, 0, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 5, 1, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 5, 2, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 5, 4, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 5, 5, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 5, 6, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 9, 0, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 9, 1, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 9, 2, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 10, 0, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 10, 1, S("can't happen"));
    test(S("abcde"), 6, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 0, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 0, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 0, 10, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 0, 19, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 0, 20, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 0, 21, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 1, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 1, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 1, 9, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 1, 18, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 1, 19, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 1, 20, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 10, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 10, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 10, 5, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 10, 9, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 10, 10, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 10, 11, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 19, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 19, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 19, 2, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 20, 0, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 20, 1, S("can't happen"));
    test(S("abcde"), 6, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghij"), 0, S(""), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S(""), 0, 1, S("abcdefghij"));
    test(S("abcdefghij"), 0, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghij"), 0, S("12345"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 0, 1, S("1abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 0, 2, S("12abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 0, 4, S("1234abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 0, 5, S("12345abcdefghij"));
}

template <class S>
void test12()
{
    test(S("abcdefghij"), 0, S("12345"), 0, 6, S("12345abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 1, 1, S("2abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 1, 2, S("23abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 1, 3, S("234abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 1, 4, S("2345abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 1, 5, S("2345abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 2, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 2, 1, S("3abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 2, 2, S("34abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 2, 3, S("345abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 2, 4, S("345abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 4, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 4, 1, S("5abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 4, 2, S("5abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 5, 1, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghij"), 0, S("1234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 0, 1, S("1abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 0, 5, S("12345abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 0, 9, S("123456789abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 0, 10, S("1234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 0, 11, S("1234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 1, 1, S("2abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 1, 4, S("2345abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 1, 8, S("23456789abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 1, 9, S("234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 1, 10, S("234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 5, 1, S("6abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 5, 2, S("67abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 5, 4, S("6789abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 5, 5, S("67890abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 5, 6, S("67890abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 9, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 9, 1, S("0abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 9, 2, S("0abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 10, 1, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 0, 1, S("1abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 0, 10, S("1234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 0, 19, S("1234567890123456789abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 0, 20, S("12345678901234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 0, 21, S("12345678901234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 1, 1, S("2abcdefghij"));
}

template <class S>
void test13()
{
    test(S("abcdefghij"), 0, S("12345678901234567890"), 1, 9, S("234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 1, 18, S("234567890123456789abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 1, 19, S("2345678901234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 1, 20, S("2345678901234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 10, 1, S("1abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 10, 5, S("12345abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 10, 9, S("123456789abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 10, 10, S("1234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 10, 11, S("1234567890abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 19, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 19, 1, S("0abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 19, 2, S("0abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 20, 0, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 20, 1, S("abcdefghij"));
    test(S("abcdefghij"), 0, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghij"), 1, S(""), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S(""), 0, 1, S("abcdefghij"));
    test(S("abcdefghij"), 1, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghij"), 1, S("12345"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 0, 1, S("a1bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 0, 2, S("a12bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 0, 4, S("a1234bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 0, 5, S("a12345bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 0, 6, S("a12345bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 1, 1, S("a2bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 1, 2, S("a23bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 1, 3, S("a234bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 1, 4, S("a2345bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 1, 5, S("a2345bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 2, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 2, 1, S("a3bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 2, 2, S("a34bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 2, 3, S("a345bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 2, 4, S("a345bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 4, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 4, 1, S("a5bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 4, 2, S("a5bcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 5, 1, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghij"), 1, S("1234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 0, 1, S("a1bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 0, 5, S("a12345bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 0, 9, S("a123456789bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 0, 10, S("a1234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 0, 11, S("a1234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 1, 1, S("a2bcdefghij"));
}

template <class S>
void test14()
{
    test(S("abcdefghij"), 1, S("1234567890"), 1, 4, S("a2345bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 1, 8, S("a23456789bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 1, 9, S("a234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 1, 10, S("a234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 5, 1, S("a6bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 5, 2, S("a67bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 5, 4, S("a6789bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 5, 5, S("a67890bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 5, 6, S("a67890bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 9, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 9, 1, S("a0bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 9, 2, S("a0bcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 10, 1, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 0, 1, S("a1bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 0, 10, S("a1234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 0, 19, S("a1234567890123456789bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 0, 20, S("a12345678901234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 0, 21, S("a12345678901234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 1, 1, S("a2bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 1, 9, S("a234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 1, 18, S("a234567890123456789bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 1, 19, S("a2345678901234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 1, 20, S("a2345678901234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 10, 1, S("a1bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 10, 5, S("a12345bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 10, 9, S("a123456789bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 10, 10, S("a1234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 10, 11, S("a1234567890bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 19, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 19, 1, S("a0bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 19, 2, S("a0bcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 20, 0, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 20, 1, S("abcdefghij"));
    test(S("abcdefghij"), 1, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghij"), 5, S(""), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S(""), 0, 1, S("abcdefghij"));
    test(S("abcdefghij"), 5, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghij"), 5, S("12345"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345"), 0, 1, S("abcde1fghij"));
    test(S("abcdefghij"), 5, S("12345"), 0, 2, S("abcde12fghij"));
    test(S("abcdefghij"), 5, S("12345"), 0, 4, S("abcde1234fghij"));
    test(S("abcdefghij"), 5, S("12345"), 0, 5, S("abcde12345fghij"));
    test(S("abcdefghij"), 5, S("12345"), 0, 6, S("abcde12345fghij"));
    test(S("abcdefghij"), 5, S("12345"), 1, 0, S("abcdefghij"));
}

template <class S>
void test15()
{
    test(S("abcdefghij"), 5, S("12345"), 1, 1, S("abcde2fghij"));
    test(S("abcdefghij"), 5, S("12345"), 1, 2, S("abcde23fghij"));
    test(S("abcdefghij"), 5, S("12345"), 1, 3, S("abcde234fghij"));
    test(S("abcdefghij"), 5, S("12345"), 1, 4, S("abcde2345fghij"));
    test(S("abcdefghij"), 5, S("12345"), 1, 5, S("abcde2345fghij"));
    test(S("abcdefghij"), 5, S("12345"), 2, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345"), 2, 1, S("abcde3fghij"));
    test(S("abcdefghij"), 5, S("12345"), 2, 2, S("abcde34fghij"));
    test(S("abcdefghij"), 5, S("12345"), 2, 3, S("abcde345fghij"));
    test(S("abcdefghij"), 5, S("12345"), 2, 4, S("abcde345fghij"));
    test(S("abcdefghij"), 5, S("12345"), 4, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345"), 4, 1, S("abcde5fghij"));
    test(S("abcdefghij"), 5, S("12345"), 4, 2, S("abcde5fghij"));
    test(S("abcdefghij"), 5, S("12345"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345"), 5, 1, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghij"), 5, S("1234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 0, 1, S("abcde1fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 0, 5, S("abcde12345fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 0, 9, S("abcde123456789fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 0, 10, S("abcde1234567890fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 0, 11, S("abcde1234567890fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 1, 1, S("abcde2fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 1, 4, S("abcde2345fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 1, 8, S("abcde23456789fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 1, 9, S("abcde234567890fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 1, 10, S("abcde234567890fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 5, 1, S("abcde6fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 5, 2, S("abcde67fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 5, 4, S("abcde6789fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 5, 5, S("abcde67890fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 5, 6, S("abcde67890fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 9, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 9, 1, S("abcde0fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 9, 2, S("abcde0fghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 10, 1, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 0, 1, S("abcde1fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 0, 10, S("abcde1234567890fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 0, 19, S("abcde1234567890123456789fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 0, 20, S("abcde12345678901234567890fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 0, 21, S("abcde12345678901234567890fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 1, 1, S("abcde2fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 1, 9, S("abcde234567890fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 1, 18, S("abcde234567890123456789fghij"));
}

template <class S>
void test16()
{
    test(S("abcdefghij"), 5, S("12345678901234567890"), 1, 19, S("abcde2345678901234567890fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 1, 20, S("abcde2345678901234567890fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 10, 1, S("abcde1fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 10, 5, S("abcde12345fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 10, 9, S("abcde123456789fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 10, 10, S("abcde1234567890fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 10, 11, S("abcde1234567890fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 19, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 19, 1, S("abcde0fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 19, 2, S("abcde0fghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 20, 0, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 20, 1, S("abcdefghij"));
    test(S("abcdefghij"), 5, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghij"), 9, S(""), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S(""), 0, 1, S("abcdefghij"));
    test(S("abcdefghij"), 9, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghij"), 9, S("12345"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345"), 0, 1, S("abcdefghi1j"));
    test(S("abcdefghij"), 9, S("12345"), 0, 2, S("abcdefghi12j"));
    test(S("abcdefghij"), 9, S("12345"), 0, 4, S("abcdefghi1234j"));
    test(S("abcdefghij"), 9, S("12345"), 0, 5, S("abcdefghi12345j"));
    test(S("abcdefghij"), 9, S("12345"), 0, 6, S("abcdefghi12345j"));
    test(S("abcdefghij"), 9, S("12345"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345"), 1, 1, S("abcdefghi2j"));
    test(S("abcdefghij"), 9, S("12345"), 1, 2, S("abcdefghi23j"));
    test(S("abcdefghij"), 9, S("12345"), 1, 3, S("abcdefghi234j"));
    test(S("abcdefghij"), 9, S("12345"), 1, 4, S("abcdefghi2345j"));
    test(S("abcdefghij"), 9, S("12345"), 1, 5, S("abcdefghi2345j"));
    test(S("abcdefghij"), 9, S("12345"), 2, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345"), 2, 1, S("abcdefghi3j"));
    test(S("abcdefghij"), 9, S("12345"), 2, 2, S("abcdefghi34j"));
    test(S("abcdefghij"), 9, S("12345"), 2, 3, S("abcdefghi345j"));
    test(S("abcdefghij"), 9, S("12345"), 2, 4, S("abcdefghi345j"));
    test(S("abcdefghij"), 9, S("12345"), 4, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345"), 4, 1, S("abcdefghi5j"));
    test(S("abcdefghij"), 9, S("12345"), 4, 2, S("abcdefghi5j"));
    test(S("abcdefghij"), 9, S("12345"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345"), 5, 1, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghij"), 9, S("1234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("1234567890"), 0, 1, S("abcdefghi1j"));
    test(S("abcdefghij"), 9, S("1234567890"), 0, 5, S("abcdefghi12345j"));
    test(S("abcdefghij"), 9, S("1234567890"), 0, 9, S("abcdefghi123456789j"));
    test(S("abcdefghij"), 9, S("1234567890"), 0, 10, S("abcdefghi1234567890j"));
    test(S("abcdefghij"), 9, S("1234567890"), 0, 11, S("abcdefghi1234567890j"));
    test(S("abcdefghij"), 9, S("1234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("1234567890"), 1, 1, S("abcdefghi2j"));
    test(S("abcdefghij"), 9, S("1234567890"), 1, 4, S("abcdefghi2345j"));
    test(S("abcdefghij"), 9, S("1234567890"), 1, 8, S("abcdefghi23456789j"));
}

template <class S>
void test17()
{
    test(S("abcdefghij"), 9, S("1234567890"), 1, 9, S("abcdefghi234567890j"));
    test(S("abcdefghij"), 9, S("1234567890"), 1, 10, S("abcdefghi234567890j"));
    test(S("abcdefghij"), 9, S("1234567890"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("1234567890"), 5, 1, S("abcdefghi6j"));
    test(S("abcdefghij"), 9, S("1234567890"), 5, 2, S("abcdefghi67j"));
    test(S("abcdefghij"), 9, S("1234567890"), 5, 4, S("abcdefghi6789j"));
    test(S("abcdefghij"), 9, S("1234567890"), 5, 5, S("abcdefghi67890j"));
    test(S("abcdefghij"), 9, S("1234567890"), 5, 6, S("abcdefghi67890j"));
    test(S("abcdefghij"), 9, S("1234567890"), 9, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("1234567890"), 9, 1, S("abcdefghi0j"));
    test(S("abcdefghij"), 9, S("1234567890"), 9, 2, S("abcdefghi0j"));
    test(S("abcdefghij"), 9, S("1234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("1234567890"), 10, 1, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 0, 1, S("abcdefghi1j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 0, 10, S("abcdefghi1234567890j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 0, 19, S("abcdefghi1234567890123456789j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 0, 20, S("abcdefghi12345678901234567890j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 0, 21, S("abcdefghi12345678901234567890j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 1, 1, S("abcdefghi2j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 1, 9, S("abcdefghi234567890j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 1, 18, S("abcdefghi234567890123456789j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 1, 19, S("abcdefghi2345678901234567890j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 1, 20, S("abcdefghi2345678901234567890j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 10, 1, S("abcdefghi1j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 10, 5, S("abcdefghi12345j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 10, 9, S("abcdefghi123456789j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 10, 10, S("abcdefghi1234567890j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 10, 11, S("abcdefghi1234567890j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 19, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 19, 1, S("abcdefghi0j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 19, 2, S("abcdefghi0j"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 20, 0, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 20, 1, S("abcdefghij"));
    test(S("abcdefghij"), 9, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghij"), 10, S(""), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S(""), 0, 1, S("abcdefghij"));
    test(S("abcdefghij"), 10, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghij"), 10, S("12345"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345"), 0, 1, S("abcdefghij1"));
    test(S("abcdefghij"), 10, S("12345"), 0, 2, S("abcdefghij12"));
    test(S("abcdefghij"), 10, S("12345"), 0, 4, S("abcdefghij1234"));
    test(S("abcdefghij"), 10, S("12345"), 0, 5, S("abcdefghij12345"));
    test(S("abcdefghij"), 10, S("12345"), 0, 6, S("abcdefghij12345"));
    test(S("abcdefghij"), 10, S("12345"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345"), 1, 1, S("abcdefghij2"));
    test(S("abcdefghij"), 10, S("12345"), 1, 2, S("abcdefghij23"));
}

template <class S>
void test18()
{
    test(S("abcdefghij"), 10, S("12345"), 1, 3, S("abcdefghij234"));
    test(S("abcdefghij"), 10, S("12345"), 1, 4, S("abcdefghij2345"));
    test(S("abcdefghij"), 10, S("12345"), 1, 5, S("abcdefghij2345"));
    test(S("abcdefghij"), 10, S("12345"), 2, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345"), 2, 1, S("abcdefghij3"));
    test(S("abcdefghij"), 10, S("12345"), 2, 2, S("abcdefghij34"));
    test(S("abcdefghij"), 10, S("12345"), 2, 3, S("abcdefghij345"));
    test(S("abcdefghij"), 10, S("12345"), 2, 4, S("abcdefghij345"));
    test(S("abcdefghij"), 10, S("12345"), 4, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345"), 4, 1, S("abcdefghij5"));
    test(S("abcdefghij"), 10, S("12345"), 4, 2, S("abcdefghij5"));
    test(S("abcdefghij"), 10, S("12345"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345"), 5, 1, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghij"), 10, S("1234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("1234567890"), 0, 1, S("abcdefghij1"));
    test(S("abcdefghij"), 10, S("1234567890"), 0, 5, S("abcdefghij12345"));
    test(S("abcdefghij"), 10, S("1234567890"), 0, 9, S("abcdefghij123456789"));
    test(S("abcdefghij"), 10, S("1234567890"), 0, 10, S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, S("1234567890"), 0, 11, S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, S("1234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("1234567890"), 1, 1, S("abcdefghij2"));
    test(S("abcdefghij"), 10, S("1234567890"), 1, 4, S("abcdefghij2345"));
    test(S("abcdefghij"), 10, S("1234567890"), 1, 8, S("abcdefghij23456789"));
    test(S("abcdefghij"), 10, S("1234567890"), 1, 9, S("abcdefghij234567890"));
    test(S("abcdefghij"), 10, S("1234567890"), 1, 10, S("abcdefghij234567890"));
    test(S("abcdefghij"), 10, S("1234567890"), 5, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("1234567890"), 5, 1, S("abcdefghij6"));
    test(S("abcdefghij"), 10, S("1234567890"), 5, 2, S("abcdefghij67"));
    test(S("abcdefghij"), 10, S("1234567890"), 5, 4, S("abcdefghij6789"));
    test(S("abcdefghij"), 10, S("1234567890"), 5, 5, S("abcdefghij67890"));
    test(S("abcdefghij"), 10, S("1234567890"), 5, 6, S("abcdefghij67890"));
    test(S("abcdefghij"), 10, S("1234567890"), 9, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("1234567890"), 9, 1, S("abcdefghij0"));
    test(S("abcdefghij"), 10, S("1234567890"), 9, 2, S("abcdefghij0"));
    test(S("abcdefghij"), 10, S("1234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("1234567890"), 10, 1, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 0, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 0, 1, S("abcdefghij1"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 0, 10, S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 0, 19, S("abcdefghij1234567890123456789"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 0, 20, S("abcdefghij12345678901234567890"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 0, 21, S("abcdefghij12345678901234567890"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 1, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 1, 1, S("abcdefghij2"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 1, 9, S("abcdefghij234567890"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 1, 18, S("abcdefghij234567890123456789"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 1, 19, S("abcdefghij2345678901234567890"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 1, 20, S("abcdefghij2345678901234567890"));
}

template <class S>
void test19()
{
    test(S("abcdefghij"), 10, S("12345678901234567890"), 10, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 10, 1, S("abcdefghij1"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 10, 5, S("abcdefghij12345"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 10, 9, S("abcdefghij123456789"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 10, 10, S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 10, 11, S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 19, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 19, 1, S("abcdefghij0"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 19, 2, S("abcdefghij0"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 20, 0, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 20, 1, S("abcdefghij"));
    test(S("abcdefghij"), 10, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S(""), 0, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S(""), 0, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 0, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 0, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 0, 2, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 0, 4, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 0, 5, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 0, 6, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 1, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 1, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 1, 2, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 1, 3, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 1, 4, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 1, 5, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 2, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 2, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 2, 2, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 2, 3, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 2, 4, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 4, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 4, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 4, 2, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 5, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 5, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 0, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 0, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 0, 5, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 0, 9, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 0, 10, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 0, 11, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 1, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 1, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 1, 4, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 1, 8, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 1, 9, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 1, 10, S("can't happen"));
}

template <class S>
void test20()
{
    test(S("abcdefghij"), 11, S("1234567890"), 5, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 5, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 5, 2, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 5, 4, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 5, 5, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 5, 6, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 9, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 9, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 9, 2, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 10, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 10, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 0, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 0, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 0, 10, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 0, 19, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 0, 20, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 0, 21, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 1, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 1, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 1, 9, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 1, 18, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 1, 19, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 1, 20, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 10, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 10, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 10, 5, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 10, 9, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 10, 10, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 10, 11, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 19, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 19, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 19, 2, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 20, 0, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 20, 1, S("can't happen"));
    test(S("abcdefghij"), 11, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, S(""), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S(""), 0, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 0, 1, S("1abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 0, 2, S("12abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 0, 4, S("1234abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 0, 5, S("12345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 0, 6, S("12345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 1, 1, S("2abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 1, 2, S("23abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 1, 3, S("234abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 1, 4, S("2345abcdefghijklmnopqrst"));
}

template <class S>
void test21()
{
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 1, 5, S("2345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 2, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 2, 1, S("3abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 2, 2, S("34abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 2, 3, S("345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 2, 4, S("345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 4, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 4, 1, S("5abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 4, 2, S("5abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 5, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 0, 1, S("1abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 0, 5, S("12345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 0, 9, S("123456789abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 0, 10, S("1234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 0, 11, S("1234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 1, 1, S("2abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 1, 4, S("2345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 1, 8, S("23456789abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 1, 9, S("234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 1, 10, S("234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 5, 1, S("6abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 5, 2, S("67abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 5, 4, S("6789abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 5, 5, S("67890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 5, 6, S("67890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 9, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 9, 1, S("0abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 9, 2, S("0abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 10, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 0, 1, S("1abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 0, 10, S("1234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 0, 19, S("1234567890123456789abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 0, 20, S("12345678901234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 0, 21, S("12345678901234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 1, 1, S("2abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 1, 9, S("234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 1, 18, S("234567890123456789abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 1, 19, S("2345678901234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 1, 20, S("2345678901234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 10, 1, S("1abcdefghijklmnopqrst"));
}

template <class S>
void test22()
{
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 10, 5, S("12345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 10, 9, S("123456789abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 10, 10, S("1234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 10, 11, S("1234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 19, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 19, 1, S("0abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 19, 2, S("0abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 20, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 20, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 1, S(""), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S(""), 0, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 0, 1, S("a1bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 0, 2, S("a12bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 0, 4, S("a1234bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 0, 5, S("a12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 0, 6, S("a12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 1, 1, S("a2bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 1, 2, S("a23bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 1, 3, S("a234bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 1, 4, S("a2345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 1, 5, S("a2345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 2, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 2, 1, S("a3bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 2, 2, S("a34bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 2, 3, S("a345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 2, 4, S("a345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 4, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 4, 1, S("a5bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 4, 2, S("a5bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 5, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 0, 1, S("a1bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 0, 5, S("a12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 0, 9, S("a123456789bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 0, 10, S("a1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 0, 11, S("a1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 1, 1, S("a2bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 1, 4, S("a2345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 1, 8, S("a23456789bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 1, 9, S("a234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 1, 10, S("a234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 5, 1, S("a6bcdefghijklmnopqrst"));
}

template <class S>
void test23()
{
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 5, 2, S("a67bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 5, 4, S("a6789bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 5, 5, S("a67890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 5, 6, S("a67890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 9, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 9, 1, S("a0bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 9, 2, S("a0bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 10, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 0, 1, S("a1bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 0, 10, S("a1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 0, 19, S("a1234567890123456789bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 0, 20, S("a12345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 0, 21, S("a12345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 1, 1, S("a2bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 1, 9, S("a234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 1, 18, S("a234567890123456789bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 1, 19, S("a2345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 1, 20, S("a2345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 10, 1, S("a1bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 10, 5, S("a12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 10, 9, S("a123456789bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 10, 10, S("a1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 10, 11, S("a1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 19, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 19, 1, S("a0bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 19, 2, S("a0bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 20, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 20, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 10, S(""), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S(""), 0, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 0, 1, S("abcdefghij1klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 0, 2, S("abcdefghij12klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 0, 4, S("abcdefghij1234klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 0, 5, S("abcdefghij12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 0, 6, S("abcdefghij12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 1, 1, S("abcdefghij2klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 1, 2, S("abcdefghij23klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 1, 3, S("abcdefghij234klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 1, 4, S("abcdefghij2345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 1, 5, S("abcdefghij2345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 2, 0, S("abcdefghijklmnopqrst"));
}

template <class S>
void test24()
{
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 2, 1, S("abcdefghij3klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 2, 2, S("abcdefghij34klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 2, 3, S("abcdefghij345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 2, 4, S("abcdefghij345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 4, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 4, 1, S("abcdefghij5klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 4, 2, S("abcdefghij5klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 5, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 0, 1, S("abcdefghij1klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 0, 5, S("abcdefghij12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 0, 9, S("abcdefghij123456789klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 0, 10, S("abcdefghij1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 0, 11, S("abcdefghij1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 1, 1, S("abcdefghij2klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 1, 4, S("abcdefghij2345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 1, 8, S("abcdefghij23456789klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 1, 9, S("abcdefghij234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 1, 10, S("abcdefghij234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 5, 1, S("abcdefghij6klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 5, 2, S("abcdefghij67klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 5, 4, S("abcdefghij6789klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 5, 5, S("abcdefghij67890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 5, 6, S("abcdefghij67890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 9, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 9, 1, S("abcdefghij0klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 9, 2, S("abcdefghij0klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 10, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 0, 1, S("abcdefghij1klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 0, 10, S("abcdefghij1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 0, 19, S("abcdefghij1234567890123456789klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 0, 20, S("abcdefghij12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 0, 21, S("abcdefghij12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 1, 1, S("abcdefghij2klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 1, 9, S("abcdefghij234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 1, 18, S("abcdefghij234567890123456789klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 1, 19, S("abcdefghij2345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 1, 20, S("abcdefghij2345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 10, 1, S("abcdefghij1klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 10, 5, S("abcdefghij12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 10, 9, S("abcdefghij123456789klmnopqrst"));
}

template <class S>
void test25()
{
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 10, 10, S("abcdefghij1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 10, 11, S("abcdefghij1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 19, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 19, 1, S("abcdefghij0klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 19, 2, S("abcdefghij0klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 20, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 20, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 19, S(""), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S(""), 0, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 0, 1, S("abcdefghijklmnopqrs1t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 0, 2, S("abcdefghijklmnopqrs12t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 0, 4, S("abcdefghijklmnopqrs1234t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 0, 5, S("abcdefghijklmnopqrs12345t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 0, 6, S("abcdefghijklmnopqrs12345t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 1, 1, S("abcdefghijklmnopqrs2t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 1, 2, S("abcdefghijklmnopqrs23t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 1, 3, S("abcdefghijklmnopqrs234t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 1, 4, S("abcdefghijklmnopqrs2345t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 1, 5, S("abcdefghijklmnopqrs2345t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 2, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 2, 1, S("abcdefghijklmnopqrs3t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 2, 2, S("abcdefghijklmnopqrs34t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 2, 3, S("abcdefghijklmnopqrs345t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 2, 4, S("abcdefghijklmnopqrs345t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 4, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 4, 1, S("abcdefghijklmnopqrs5t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 4, 2, S("abcdefghijklmnopqrs5t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 5, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 0, 1, S("abcdefghijklmnopqrs1t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 0, 5, S("abcdefghijklmnopqrs12345t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 0, 9, S("abcdefghijklmnopqrs123456789t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 0, 10, S("abcdefghijklmnopqrs1234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 0, 11, S("abcdefghijklmnopqrs1234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 1, 1, S("abcdefghijklmnopqrs2t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 1, 4, S("abcdefghijklmnopqrs2345t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 1, 8, S("abcdefghijklmnopqrs23456789t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 1, 9, S("abcdefghijklmnopqrs234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 1, 10, S("abcdefghijklmnopqrs234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 5, 1, S("abcdefghijklmnopqrs6t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 5, 2, S("abcdefghijklmnopqrs67t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 5, 4, S("abcdefghijklmnopqrs6789t"));
}

template <class S>
void test26()
{
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 5, 5, S("abcdefghijklmnopqrs67890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 5, 6, S("abcdefghijklmnopqrs67890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 9, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 9, 1, S("abcdefghijklmnopqrs0t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 9, 2, S("abcdefghijklmnopqrs0t"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 10, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 0, 1, S("abcdefghijklmnopqrs1t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 0, 10, S("abcdefghijklmnopqrs1234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 0, 19, S("abcdefghijklmnopqrs1234567890123456789t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 0, 20, S("abcdefghijklmnopqrs12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 0, 21, S("abcdefghijklmnopqrs12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 1, 1, S("abcdefghijklmnopqrs2t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 1, 9, S("abcdefghijklmnopqrs234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 1, 18, S("abcdefghijklmnopqrs234567890123456789t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 1, 19, S("abcdefghijklmnopqrs2345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 1, 20, S("abcdefghijklmnopqrs2345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 10, 1, S("abcdefghijklmnopqrs1t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 10, 5, S("abcdefghijklmnopqrs12345t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 10, 9, S("abcdefghijklmnopqrs123456789t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 10, 10, S("abcdefghijklmnopqrs1234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 10, 11, S("abcdefghijklmnopqrs1234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 19, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 19, 1, S("abcdefghijklmnopqrs0t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 19, 2, S("abcdefghijklmnopqrs0t"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 20, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 20, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 20, S(""), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S(""), 0, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 0, 1, S("abcdefghijklmnopqrst1"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 0, 2, S("abcdefghijklmnopqrst12"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 0, 4, S("abcdefghijklmnopqrst1234"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 0, 5, S("abcdefghijklmnopqrst12345"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 0, 6, S("abcdefghijklmnopqrst12345"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 1, 1, S("abcdefghijklmnopqrst2"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 1, 2, S("abcdefghijklmnopqrst23"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 1, 3, S("abcdefghijklmnopqrst234"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 1, 4, S("abcdefghijklmnopqrst2345"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 1, 5, S("abcdefghijklmnopqrst2345"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 2, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 2, 1, S("abcdefghijklmnopqrst3"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 2, 2, S("abcdefghijklmnopqrst34"));
}

template <class S>
void test27()
{
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 2, 3, S("abcdefghijklmnopqrst345"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 2, 4, S("abcdefghijklmnopqrst345"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 4, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 4, 1, S("abcdefghijklmnopqrst5"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 4, 2, S("abcdefghijklmnopqrst5"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 5, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 0, 1, S("abcdefghijklmnopqrst1"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 0, 5, S("abcdefghijklmnopqrst12345"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 0, 9, S("abcdefghijklmnopqrst123456789"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 0, 10, S("abcdefghijklmnopqrst1234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 0, 11, S("abcdefghijklmnopqrst1234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 1, 1, S("abcdefghijklmnopqrst2"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 1, 4, S("abcdefghijklmnopqrst2345"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 1, 8, S("abcdefghijklmnopqrst23456789"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 1, 9, S("abcdefghijklmnopqrst234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 1, 10, S("abcdefghijklmnopqrst234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 5, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 5, 1, S("abcdefghijklmnopqrst6"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 5, 2, S("abcdefghijklmnopqrst67"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 5, 4, S("abcdefghijklmnopqrst6789"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 5, 5, S("abcdefghijklmnopqrst67890"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 5, 6, S("abcdefghijklmnopqrst67890"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 9, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 9, 1, S("abcdefghijklmnopqrst0"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 9, 2, S("abcdefghijklmnopqrst0"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 10, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 0, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 0, 1, S("abcdefghijklmnopqrst1"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 0, 10, S("abcdefghijklmnopqrst1234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 0, 19, S("abcdefghijklmnopqrst1234567890123456789"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 0, 20, S("abcdefghijklmnopqrst12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 0, 21, S("abcdefghijklmnopqrst12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 1, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 1, 1, S("abcdefghijklmnopqrst2"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 1, 9, S("abcdefghijklmnopqrst234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 1, 18, S("abcdefghijklmnopqrst234567890123456789"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 1, 19, S("abcdefghijklmnopqrst2345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 1, 20, S("abcdefghijklmnopqrst2345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 10, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 10, 1, S("abcdefghijklmnopqrst1"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 10, 5, S("abcdefghijklmnopqrst12345"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 10, 9, S("abcdefghijklmnopqrst123456789"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 10, 10, S("abcdefghijklmnopqrst1234567890"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 10, 11, S("abcdefghijklmnopqrst1234567890"));
}

template <class S>
void test28()
{
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 19, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 19, 1, S("abcdefghijklmnopqrst0"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 19, 2, S("abcdefghijklmnopqrst0"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 20, 0, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 20, 1, S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, S("12345678901234567890"), 21, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S(""), 0, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S(""), 0, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S(""), 1, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 0, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 0, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 0, 2, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 0, 4, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 0, 5, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 0, 6, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 1, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 1, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 1, 2, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 1, 3, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 1, 4, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 1, 5, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 2, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 2, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 2, 2, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 2, 3, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 2, 4, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 4, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 4, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 4, 2, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 5, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 5, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345"), 6, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 0, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 0, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 0, 5, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 0, 9, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 0, 10, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 0, 11, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 1, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 1, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 1, 4, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 1, 8, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 1, 9, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 1, 10, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 5, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 5, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 5, 2, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 5, 4, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 5, 5, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 5, 6, S("can't happen"));
}

template <class S>
void test29()
{
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 9, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 9, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 9, 2, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 10, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 10, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("1234567890"), 11, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 0, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 0, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 0, 10, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 0, 19, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 0, 20, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 0, 21, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 1, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 1, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 1, 9, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 1, 18, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 1, 19, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 1, 20, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 10, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 10, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 10, 5, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 10, 9, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 10, 10, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 10, 11, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 19, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 19, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 19, 2, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 20, 0, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 20, 1, S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, S("12345678901234567890"), 21, 0, S("can't happen"));
}

template <class S>
void test30()
{
    test_npos(S(""), 0, S("12345678901234567890"),  0, S("12345678901234567890"));
    test_npos(S(""), 0, S("12345678901234567890"),  1, S( "2345678901234567890"));
    test_npos(S(""), 0, S("12345678901234567890"),  2, S(  "345678901234567890"));
    test_npos(S(""), 0, S("12345678901234567890"),  3, S(   "45678901234567890"));
    test_npos(S(""), 0, S("12345678901234567890"),  5, S(     "678901234567890"));
    test_npos(S(""), 0, S("12345678901234567890"), 10, S(          "1234567890"));
    test_npos(S(""), 0, S("12345678901234567890"), 21, S("can't happen"));
    test_npos(S("abcdefghijklmnopqrst"), 10, S("12345"), 0, S("abcdefghij12345klmnopqrst"));
    test_npos(S("abcdefghijklmnopqrst"), 10, S("12345"), 1, S("abcdefghij2345klmnopqrst"));
    test_npos(S("abcdefghijklmnopqrst"), 10, S("12345"), 3, S("abcdefghij45klmnopqrst"));
    test_npos(S("abcdefghijklmnopqrst"), 10, S("12345"), 5, S("abcdefghijklmnopqrst"));
    test_npos(S("abcdefghijklmnopqrst"), 10, S("12345"), 6, S("can't happen"));
}

int main()
{
    {
    typedef std::string S;
    test0<S>();
    test1<S>();
    test2<S>();
    test3<S>();
    test4<S>();
    test5<S>();
    test6<S>();
    test7<S>();
    test8<S>();
    test9<S>();
    test10<S>();
    test11<S>();
    test12<S>();
    test13<S>();
    test14<S>();
    test15<S>();
    test16<S>();
    test17<S>();
    test18<S>();
    test19<S>();
    test20<S>();
    test21<S>();
    test22<S>();
    test23<S>();
    test24<S>();
    test25<S>();
    test26<S>();
    test27<S>();
    test28<S>();
    test29<S>();
    test30<S>();
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test0<S>();
    test1<S>();
    test2<S>();
    test3<S>();
    test4<S>();
    test5<S>();
    test6<S>();
    test7<S>();
    test8<S>();
    test9<S>();
    test10<S>();
    test11<S>();
    test12<S>();
    test13<S>();
    test14<S>();
    test15<S>();
    test16<S>();
    test17<S>();
    test18<S>();
    test19<S>();
    test20<S>();
    test21<S>();
    test22<S>();
    test23<S>();
    test24<S>();
    test25<S>();
    test26<S>();
    test27<S>();
    test28<S>();
    test29<S>();
    test30<S>();
    }
#endif
}
