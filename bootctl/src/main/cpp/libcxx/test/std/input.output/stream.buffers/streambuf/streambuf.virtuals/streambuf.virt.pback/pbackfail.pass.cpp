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

// int_type pbackfail(int_type c = traits::eof());

#include <streambuf>
#include <cassert>

int pbackfail_called = 0;

struct test
    : public std::basic_streambuf<char>
{
    test() {}
};

int main()
{
    test t;
    assert(t.sputbackc('A') == -1);
}
