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

// void setg(char_type* gbeg, char_type* gnext, char_type* gend);

#include <streambuf>
#include <cassert>

template <class CharT>
struct test
    : public std::basic_streambuf<CharT>
{
    typedef std::basic_streambuf<CharT> base;

    test() {}

    void setg(CharT* gbeg, CharT* gnext, CharT* gend)
    {
        base::setg(gbeg, gnext, gend);
        assert(base::eback() == gbeg);
        assert(base::gptr() == gnext);
        assert(base::egptr() == gend);
    }
};

int main()
{
    {
        test<char> t;
        char in[] = "ABC";
        t.setg(in, in+1, in+sizeof(in)/sizeof(in[0]));
    }
    {
        test<wchar_t> t;
        wchar_t in[] = L"ABC";
        t.setg(in, in+1, in+sizeof(in)/sizeof(in[0]));
    }
}
