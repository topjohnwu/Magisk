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

// streamsize sgetn(char_type* s, streamsize n);

#include <streambuf>
#include <cassert>

int xsgetn_called = 0;

struct test
    : public std::basic_streambuf<char>
{
    test() {}

protected:
    std::streamsize xsgetn(char_type*, std::streamsize)
    {
        ++xsgetn_called;
        return 10;
    }
};

int main()
{
    test t;
    assert(xsgetn_called == 0);
    assert(t.sgetn(0, 0) == 10);
    assert(xsgetn_called == 1);
}
