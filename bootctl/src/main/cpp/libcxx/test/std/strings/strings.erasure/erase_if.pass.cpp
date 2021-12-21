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

// template <class charT, class traits, class Allocator, class Predicate>
//   void erase_if(basic_string<charT, traits, Allocator>& c, Predicate pred);  

#include <string>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

template <class S, class Pred>
void
test0(S s, Pred p, S expected)
{
    ASSERT_SAME_TYPE(void, decltype(std::erase_if(s, p)));
    std::erase_if(s, p);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
}

template <typename S>
void test()
{
    auto isA = [](auto ch) { return ch == 'a';};
    auto isB = [](auto ch) { return ch == 'b';};
    auto isC = [](auto ch) { return ch == 'c';};
    auto isD = [](auto ch) { return ch == 'd';};
    auto True  = [](auto) { return true; };
    auto False = [](auto) { return false; };
    
    test0(S(""), isA, S(""));

    test0(S("a"), isA, S(""));
    test0(S("a"), isB, S("a"));

    test0(S("ab"), isA, S("b"));
    test0(S("ab"), isB, S("a"));
    test0(S("ab"), isC, S("ab"));
    test0(S("aa"), isA, S(""));
    test0(S("aa"), isC, S("aa"));

    test0(S("abc"), isA, S("bc"));
    test0(S("abc"), isB, S("ac"));
    test0(S("abc"), isC, S("ab"));
    test0(S("abc"), isD, S("abc"));

    test0(S("aab"), isA, S("b"));
    test0(S("aab"), isB, S("aa"));
    test0(S("aab"), isC, S("aab"));
    test0(S("abb"), isA, S("bb"));
    test0(S("abb"), isB, S("a"));
    test0(S("abb"), isC, S("abb"));
    test0(S("aaa"), isA, S(""));
    test0(S("aaa"), isB, S("aaa"));

    test0(S("aba"), False,  S("aba"));
    test0(S("aba"), True,   S(""));
}

int main()
{
    test<std::string>();
    test<std::basic_string<char, std::char_traits<char>, min_allocator<char>>> ();
    test<std::basic_string<char, std::char_traits<char>, test_allocator<char>>> ();
}
