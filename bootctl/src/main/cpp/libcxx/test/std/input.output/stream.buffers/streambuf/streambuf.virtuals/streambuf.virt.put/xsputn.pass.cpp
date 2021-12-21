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

// streamsize xsputn(const char_type* s, streamsize n);

#include <streambuf>
#include <cassert>

struct test
    : public std::basic_streambuf<char>
{
    typedef std::basic_streambuf<char> base;

    test() {}

    void setp(char* pbeg, char* pend)
    {
        base::setp(pbeg, pend);
    }
};

int main()
{
    {
        test t;
        char in[] = "123456";
        assert(t.sputn(in, sizeof(in)) == 0);
        char out[sizeof(in)] = {0};
        t.setp(out, out+sizeof(out));
        assert(t.sputn(in, sizeof(in)) == sizeof(in));
        assert(strcmp(in, out) == 0);
    }
}
