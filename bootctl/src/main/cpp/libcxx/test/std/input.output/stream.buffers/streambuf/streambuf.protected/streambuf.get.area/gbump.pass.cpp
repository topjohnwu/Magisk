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

// void gbump(int n);

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
    }

    void gbump(int n)
    {
        CharT* gbeg = base::eback();
        CharT* gnext = base::gptr();
        CharT* gend = base::egptr();
        base::gbump(n);
        assert(base::eback() == gbeg);
        assert(base::gptr() == gnext+n);
        assert(base::egptr() == gend);
    }
};

int main()
{
    {
        test<char> t;
        char in[] = "ABCDE";
        t.setg(in, in+1, in+sizeof(in)/sizeof(in[0]));
        t.gbump(2);
    }
    {
        test<wchar_t> t;
        wchar_t in[] = L"ABCDE";
        t.setg(in, in+1, in+sizeof(in)/sizeof(in[0]));
        t.gbump(3);
    }
}
