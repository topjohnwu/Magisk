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

// streamsize showmanyc();

#include <streambuf>
#include <cassert>

int showmanyc_called = 0;

template <class CharT>
struct test
    : public std::basic_streambuf<CharT>
{
    test() {}
};

int main()
{
    test<char> t;
    assert(t.in_avail() == 0);
}
