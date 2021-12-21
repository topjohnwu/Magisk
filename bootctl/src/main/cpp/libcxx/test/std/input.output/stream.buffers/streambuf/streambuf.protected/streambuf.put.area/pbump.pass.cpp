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

// void pbump(int n);

#include <streambuf>
#include <cassert>

template <class CharT>
struct test
    : public std::basic_streambuf<CharT>
{
    typedef std::basic_streambuf<CharT> base;

    test() {}

    void setp(CharT* pbeg, CharT* pend)
    {
        base::setp(pbeg, pend);
    }

    void pbump(int n)
    {
        CharT* pbeg = base::pbase();
        CharT* pnext = base::pptr();
        CharT* pend = base::epptr();
        base::pbump(n);
        assert(base::pbase() == pbeg);
        assert(base::pptr() == pnext+n);
        assert(base::epptr() == pend);
    }
};

int main()
{
    {
        test<char> t;
        char in[] = "ABCDE";
        t.setp(in, in+sizeof(in)/sizeof(in[0]));
        t.pbump(2);
        t.pbump(1);
    }
    {
        test<wchar_t> t;
        wchar_t in[] = L"ABCDE";
        t.setp(in, in+sizeof(in)/sizeof(in[0]));
        t.pbump(3);
        t.pbump(1);
    }
}
