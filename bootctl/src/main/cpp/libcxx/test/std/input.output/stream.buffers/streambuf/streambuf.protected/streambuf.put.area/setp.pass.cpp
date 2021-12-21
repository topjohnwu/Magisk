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

// void setp(char_type* pbeg, char_type* pend);

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
        assert(base::pbase() == pbeg);
        assert(base::pptr() == pbeg);
        assert(base::epptr() == pend);
    }
};

int main()
{
    {
        test<char> t;
        char in[] = "ABC";
        t.setp(in, in+sizeof(in)/sizeof(in[0]));
    }
    {
        test<wchar_t> t;
        wchar_t in[] = L"ABC";
        t.setp(in, in+sizeof(in)/sizeof(in[0]));
    }
}
