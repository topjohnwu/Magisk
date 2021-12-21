//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<class InputIterator>
//   basic_string& assign(InputIterator first, InputIterator last);

#include <string>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "min_allocator.h"

template <class S, class It>
void
test(S s, It first, It last, S expected)
{
    s.assign(first, last);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
}

#ifndef TEST_HAS_NO_EXCEPTIONS
template <class S, class It>
void
test_exceptions(S s, It first, It last)
{
    S aCopy = s;
    try {
        s.assign(first, last);
        assert(false);
    }
    catch (...) {}
    LIBCPP_ASSERT(s.__invariants());
    assert(s == aCopy);
}
#endif

int main()
{
    {
    typedef std::string S;
    const char* s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    test(S(), s, s, S());
    test(S(), s, s+1, S("A"));
    test(S(), s, s+10, S("ABCDEFGHIJ"));
    test(S(), s, s+52, S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345"), s, s, S());
    test(S("12345"), s, s+1, S("A"));
    test(S("12345"), s, s+10, S("ABCDEFGHIJ"));
    test(S("12345"), s, s+52, S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("1234567890"), s, s, S());
    test(S("1234567890"), s, s+1, S("A"));
    test(S("1234567890"), s, s+10, S("ABCDEFGHIJ"));
    test(S("1234567890"), s, s+52, S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345678901234567890"), s, s, S());
    test(S("12345678901234567890"), s, s+1, S("A"));
    test(S("12345678901234567890"), s, s+10, S("ABCDEFGHIJ"));
    test(S("12345678901234567890"), s, s+52,
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s), S());
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+1), S("A"));
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S());
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("A"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S());
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("A"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S());
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("A"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    const char* s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    test(S(), s, s, S());
    test(S(), s, s+1, S("A"));
    test(S(), s, s+10, S("ABCDEFGHIJ"));
    test(S(), s, s+52, S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345"), s, s, S());
    test(S("12345"), s, s+1, S("A"));
    test(S("12345"), s, s+10, S("ABCDEFGHIJ"));
    test(S("12345"), s, s+52, S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("1234567890"), s, s, S());
    test(S("1234567890"), s, s+1, S("A"));
    test(S("1234567890"), s, s+10, S("ABCDEFGHIJ"));
    test(S("1234567890"), s, s+52, S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345678901234567890"), s, s, S());
    test(S("12345678901234567890"), s, s+1, S("A"));
    test(S("12345678901234567890"), s, s+10, S("ABCDEFGHIJ"));
    test(S("12345678901234567890"), s, s+52,
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s), S());
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+1), S("A"));
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S());
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("A"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S());
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("A"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S());
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("A"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));
    }
#endif
#ifndef TEST_HAS_NO_EXCEPTIONS
    { // test iterator operations that throw
    typedef std::string S;
    typedef ThrowingIterator<char> TIter;
    typedef input_iterator<TIter> IIter;
    const char* s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    test_exceptions(S(), IIter(TIter(s, s+10, 4, TIter::TAIncrement)), IIter());
    test_exceptions(S(), IIter(TIter(s, s+10, 5, TIter::TADereference)), IIter());
    test_exceptions(S(), IIter(TIter(s, s+10, 6, TIter::TAComparison)), IIter());

    test_exceptions(S(), TIter(s, s+10, 4, TIter::TAIncrement), TIter());
    test_exceptions(S(), TIter(s, s+10, 5, TIter::TADereference), TIter());
    test_exceptions(S(), TIter(s, s+10, 6, TIter::TAComparison), TIter());
    }
#endif

    { // test assigning to self
    typedef std::string S;
    S s_short = "123/";
    S s_long  = "Lorem ipsum dolor sit amet, consectetur/";

    s_short.assign(s_short.begin(), s_short.end());
    assert(s_short == "123/");
    s_short.assign(s_short.begin() + 2, s_short.end());
    assert(s_short == "3/");

    s_long.assign(s_long.begin(), s_long.end());
    assert(s_long == "Lorem ipsum dolor sit amet, consectetur/");

    s_long.assign(s_long.begin() + 30, s_long.end());
    assert(s_long == "nsectetur/");
    }

    { // test assigning a different type
    typedef std::string S;
    const uint8_t p[] = "ABCD";

    S s;
    s.assign(p, p + 4);
    assert(s == "ABCD");
    }
}
