//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<class charT, class traits, class Allocator>
//   void swap(basic_string<charT,traits,Allocator>& lhs,
//             basic_string<charT,traits,Allocator>& rhs);

#include <string>
#include <stdexcept>
#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s1, S s2)
{
    S s1_ = s1;
    S s2_ = s2;
    swap(s1, s2);
    LIBCPP_ASSERT(s1.__invariants());
    LIBCPP_ASSERT(s2.__invariants());
    assert(s1 == s2_);
    assert(s2 == s1_);
}

int main()
{
    {
    typedef std::string S;
    test(S(""), S(""));
    test(S(""), S("12345"));
    test(S(""), S("1234567890"));
    test(S(""), S("12345678901234567890"));
    test(S("abcde"), S(""));
    test(S("abcde"), S("12345"));
    test(S("abcde"), S("1234567890"));
    test(S("abcde"), S("12345678901234567890"));
    test(S("abcdefghij"), S(""));
    test(S("abcdefghij"), S("12345"));
    test(S("abcdefghij"), S("1234567890"));
    test(S("abcdefghij"), S("12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), S(""));
    test(S("abcdefghijklmnopqrst"), S("12345"));
    test(S("abcdefghijklmnopqrst"), S("1234567890"));
    test(S("abcdefghijklmnopqrst"), S("12345678901234567890"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), S(""));
    test(S(""), S("12345"));
    test(S(""), S("1234567890"));
    test(S(""), S("12345678901234567890"));
    test(S("abcde"), S(""));
    test(S("abcde"), S("12345"));
    test(S("abcde"), S("1234567890"));
    test(S("abcde"), S("12345678901234567890"));
    test(S("abcdefghij"), S(""));
    test(S("abcdefghij"), S("12345"));
    test(S("abcdefghij"), S("1234567890"));
    test(S("abcdefghij"), S("12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), S(""));
    test(S("abcdefghijklmnopqrst"), S("12345"));
    test(S("abcdefghijklmnopqrst"), S("1234567890"));
    test(S("abcdefghijklmnopqrst"), S("12345678901234567890"));
    }
#endif
}
