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

// streamsize xsgetn(char_type* s, streamsize n);

#include <streambuf>
#include <cassert>

struct test
    : public std::basic_streambuf<char>
{
    typedef std::basic_streambuf<char> base;

    test() {}

    void setg(char* gbeg, char* gnext, char* gend)
    {
        base::setg(gbeg, gnext, gend);
    }
};

int main()
{
    test t;
    char input[7] = "123456";
    t.setg(input, input, input+7);
    char output[sizeof(input)] = {0};
    assert(t.sgetn(output, 10) == 7);
    assert(strcmp(input, output) == 0);
}
