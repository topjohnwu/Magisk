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

// streamsize in_avail();

#include <streambuf>
#include <cassert>

int showmanyc_called = 0;

template <class CharT>
struct test
    : public std::basic_streambuf<CharT>
{
    typedef std::basic_streambuf<CharT> base;

    test() {}

    void setg(CharT* gbeg, CharT* gnext, CharT* gend)
    {
        base::setg(gbeg, gnext, gend);
    }
protected:
    std::streamsize showmanyc()
    {
        ++showmanyc_called;
        return 5;
    }
};

int main()
{
    {
        test<char> t;
        assert(t.in_avail() == 5);
        assert(showmanyc_called == 1);
        char in[5];
        t.setg(in, in+2, in+5);
        assert(t.in_avail() == 3);
    }
}
