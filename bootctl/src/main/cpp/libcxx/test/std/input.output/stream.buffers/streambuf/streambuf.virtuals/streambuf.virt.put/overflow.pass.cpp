//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <streambuf>

// template <class charT, class traits = char_traits<charT> >
// class basic_streambuf;

// int_type overflow(int_type c = traits::eof());

#include <streambuf>
#include <cassert>

struct test
    : public std::basic_streambuf<char>
{
    test() {}
};

int main()
{
    test t;
    assert(t.sputc('A') == -1);
}
