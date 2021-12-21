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

// int_type sgetc();

#include <streambuf>
#include <cassert>

int underflow_called = 0;

struct test
    : public std::basic_streambuf<char>
{
    typedef std::basic_streambuf<char> base;

    test() {}

    void setg(char* gbeg, char* gnext, char* gend)
    {
        base::setg(gbeg, gnext, gend);
    }

protected:
    int_type underflow()
    {
        ++underflow_called;
        return 'a';
    }
};

int main()
{
    {
        test t;
        assert(underflow_called == 0);
        assert(t.sgetc() == 'a');
        assert(underflow_called == 1);
        char in[] = "ABC";
        t.setg(in, in, in+sizeof(in));
        assert(t.sgetc() == 'A');
        assert(underflow_called == 1);
        assert(t.sgetc() == 'A');
        assert(underflow_called == 1);
    }
}
