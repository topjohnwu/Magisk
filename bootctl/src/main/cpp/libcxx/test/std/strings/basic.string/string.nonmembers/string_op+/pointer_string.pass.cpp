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
//   basic_string<charT,traits,Allocator>
//   operator+(const charT* lhs, const basic_string<charT,traits,Allocator>& rhs);

// template<class charT, class traits, class Allocator>
//   basic_string<charT,traits,Allocator>&&
//   operator+(const charT* lhs, basic_string<charT,traits,Allocator>&& rhs);

#include <string>
#include <utility>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void test0(const typename S::value_type* lhs, const S& rhs, const S& x) {
  assert(lhs + rhs == x);
}

#if TEST_STD_VER >= 11
template <class S>
void test1(const typename S::value_type* lhs, S&& rhs, const S& x) {
  assert(lhs + move(rhs) == x);
}
#endif

int main() {
  {
    typedef std::string S;
    test0("", S(""), S(""));
    test0("", S("12345"), S("12345"));
    test0("", S("1234567890"), S("1234567890"));
    test0("", S("12345678901234567890"), S("12345678901234567890"));
    test0("abcde", S(""), S("abcde"));
    test0("abcde", S("12345"), S("abcde12345"));
    test0("abcde", S("1234567890"), S("abcde1234567890"));
    test0("abcde", S("12345678901234567890"), S("abcde12345678901234567890"));
    test0("abcdefghij", S(""), S("abcdefghij"));
    test0("abcdefghij", S("12345"), S("abcdefghij12345"));
    test0("abcdefghij", S("1234567890"), S("abcdefghij1234567890"));
    test0("abcdefghij", S("12345678901234567890"),
          S("abcdefghij12345678901234567890"));
    test0("abcdefghijklmnopqrst", S(""), S("abcdefghijklmnopqrst"));
    test0("abcdefghijklmnopqrst", S("12345"), S("abcdefghijklmnopqrst12345"));
    test0("abcdefghijklmnopqrst", S("1234567890"),
          S("abcdefghijklmnopqrst1234567890"));
    test0("abcdefghijklmnopqrst", S("12345678901234567890"),
          S("abcdefghijklmnopqrst12345678901234567890"));
  }

#if TEST_STD_VER >= 11
  {
    typedef std::string S;
    test1("", S(""), S(""));
    test1("", S("12345"), S("12345"));
    test1("", S("1234567890"), S("1234567890"));
    test1("", S("12345678901234567890"), S("12345678901234567890"));
    test1("abcde", S(""), S("abcde"));
    test1("abcde", S("12345"), S("abcde12345"));
    test1("abcde", S("1234567890"), S("abcde1234567890"));
    test1("abcde", S("12345678901234567890"), S("abcde12345678901234567890"));
    test1("abcdefghij", S(""), S("abcdefghij"));
    test1("abcdefghij", S("12345"), S("abcdefghij12345"));
    test1("abcdefghij", S("1234567890"), S("abcdefghij1234567890"));
    test1("abcdefghij", S("12345678901234567890"),
          S("abcdefghij12345678901234567890"));
    test1("abcdefghijklmnopqrst", S(""), S("abcdefghijklmnopqrst"));
    test1("abcdefghijklmnopqrst", S("12345"), S("abcdefghijklmnopqrst12345"));
    test1("abcdefghijklmnopqrst", S("1234567890"),
          S("abcdefghijklmnopqrst1234567890"));
    test1("abcdefghijklmnopqrst", S("12345678901234567890"),
          S("abcdefghijklmnopqrst12345678901234567890"));
  }
  {
    typedef std::basic_string<char, std::char_traits<char>,
                              min_allocator<char> >
        S;
    test0("", S(""), S(""));
    test0("", S("12345"), S("12345"));
    test0("", S("1234567890"), S("1234567890"));
    test0("", S("12345678901234567890"), S("12345678901234567890"));
    test0("abcde", S(""), S("abcde"));
    test0("abcde", S("12345"), S("abcde12345"));
    test0("abcde", S("1234567890"), S("abcde1234567890"));
    test0("abcde", S("12345678901234567890"), S("abcde12345678901234567890"));
    test0("abcdefghij", S(""), S("abcdefghij"));
    test0("abcdefghij", S("12345"), S("abcdefghij12345"));
    test0("abcdefghij", S("1234567890"), S("abcdefghij1234567890"));
    test0("abcdefghij", S("12345678901234567890"),
          S("abcdefghij12345678901234567890"));
    test0("abcdefghijklmnopqrst", S(""), S("abcdefghijklmnopqrst"));
    test0("abcdefghijklmnopqrst", S("12345"), S("abcdefghijklmnopqrst12345"));
    test0("abcdefghijklmnopqrst", S("1234567890"),
          S("abcdefghijklmnopqrst1234567890"));
    test0("abcdefghijklmnopqrst", S("12345678901234567890"),
          S("abcdefghijklmnopqrst12345678901234567890"));

    test1("", S(""), S(""));
    test1("", S("12345"), S("12345"));
    test1("", S("1234567890"), S("1234567890"));
    test1("", S("12345678901234567890"), S("12345678901234567890"));
    test1("abcde", S(""), S("abcde"));
    test1("abcde", S("12345"), S("abcde12345"));
    test1("abcde", S("1234567890"), S("abcde1234567890"));
    test1("abcde", S("12345678901234567890"), S("abcde12345678901234567890"));
    test1("abcdefghij", S(""), S("abcdefghij"));
    test1("abcdefghij", S("12345"), S("abcdefghij12345"));
    test1("abcdefghij", S("1234567890"), S("abcdefghij1234567890"));
    test1("abcdefghij", S("12345678901234567890"),
          S("abcdefghij12345678901234567890"));
    test1("abcdefghijklmnopqrst", S(""), S("abcdefghijklmnopqrst"));
    test1("abcdefghijklmnopqrst", S("12345"), S("abcdefghijklmnopqrst12345"));
    test1("abcdefghijklmnopqrst", S("1234567890"),
          S("abcdefghijklmnopqrst1234567890"));
    test1("abcdefghijklmnopqrst", S("12345678901234567890"),
          S("abcdefghijklmnopqrst12345678901234567890"));
  }
#endif
}
