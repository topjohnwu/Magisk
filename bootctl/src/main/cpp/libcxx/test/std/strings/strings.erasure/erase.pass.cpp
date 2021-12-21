//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <string>

// template <class charT, class traits, class Allocator, class U>
//   void erase(basic_string<charT, traits, Allocator>& c, const U& value);
  

#include <string>
#include <optional>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

template <class S, class U>
void
test0(S s,  U val, S expected)
{
    ASSERT_SAME_TYPE(void, decltype(std::erase(s, val)));
    std::erase(s, val);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
}

template <class S>
void test()
{

    test0(S(""), 'a', S(""));

    test0(S("a"), 'a', S(""));
    test0(S("a"), 'b', S("a"));

    test0(S("ab"), 'a', S("b"));
    test0(S("ab"), 'b', S("a"));
    test0(S("ab"), 'c', S("ab"));
    test0(S("aa"), 'a', S(""));
    test0(S("aa"), 'c', S("aa"));

    test0(S("abc"), 'a', S("bc"));
    test0(S("abc"), 'b', S("ac"));
    test0(S("abc"), 'c', S("ab"));
    test0(S("abc"), 'd', S("abc"));

    test0(S("aab"), 'a', S("b"));
    test0(S("aab"), 'b', S("aa"));
    test0(S("aab"), 'c', S("aab"));
    test0(S("abb"), 'a', S("bb"));
    test0(S("abb"), 'b', S("a"));
    test0(S("abb"), 'c', S("abb"));
    test0(S("aaa"), 'a', S(""));
    test0(S("aaa"), 'b', S("aaa"));

//  Test cross-type erasure
    using opt = std::optional<typename S::value_type>;
    test0(S("aba"), opt(),    S("aba"));
    test0(S("aba"), opt('a'), S("b"));
    test0(S("aba"), opt('b'), S("aa"));
    test0(S("aba"), opt('c'), S("aba"));
}

int main()
{
    test<std::string>();
    test<std::basic_string<char, std::char_traits<char>, min_allocator<char>>> ();
    test<std::basic_string<char, std::char_traits<char>, test_allocator<char>>> ();
}
