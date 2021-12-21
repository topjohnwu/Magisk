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

// streamsize sputn(const char_type* s, streamsize n);

#include <streambuf>
#include <cassert>

int xsputn_called = 0;

struct test
    : public std::basic_streambuf<char>
{
    test() {}

protected:
    std::streamsize xsputn(const char_type*, std::streamsize)
    {
        ++xsputn_called;
        return 5;
    }
};

int main()
{
    test t;
    assert(xsputn_called == 0);
    assert(t.sputn(0, 0) == 5);
    assert(xsputn_called == 1);
}
