//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <streambuf>

// template <class charT, class traits = char_traits<charT> >
// class basic_streambuf;

// basic_streambuf& operator=(const basic_streambuf& rhs);

#include <streambuf>
#include <cassert>

#include "platform_support.h" // locale name macros

template <class CharT>
struct test
    : public std::basic_streambuf<CharT>
{
    typedef std::basic_streambuf<CharT> base;
    test() {}

    test& operator=(const test& t)
    {
        base::operator=(t);
        assert(this->eback() == t.eback());
        assert(this->gptr()  == t.gptr());
        assert(this->egptr() == t.egptr());
        assert(this->pbase() == t.pbase());
        assert(this->pptr()  == t.pptr());
        assert(this->epptr() == t.epptr());
        assert(this->getloc() == t.getloc());
        return *this;
    }

    void setg(CharT* gbeg, CharT* gnext, CharT* gend)
    {
        base::setg(gbeg, gnext, gend);
    }
    void setp(CharT* pbeg, CharT* pend)
    {
        base::setp(pbeg, pend);
    }
};

int main()
{
    {
        test<char> t;
        test<char> t2;
        t2 = t;
    }
    {
        test<wchar_t> t;
        test<wchar_t> t2;
        t2 = t;
    }
    {
        char g1, g2, g3, p1, p3;
        test<char> t;
        t.setg(&g1, &g2, &g3);
        t.setp(&p1, &p3);
        test<char> t2;
        t2 = t;
    }
    {
        wchar_t g1, g2, g3, p1, p3;
        test<wchar_t> t;
        t.setg(&g1, &g2, &g3);
        t.setp(&p1, &p3);
        test<wchar_t> t2;
        t2 = t;
    }
    std::locale::global(std::locale(LOCALE_en_US_UTF_8));
    {
        test<char> t;
        test<char> t2;
        t2 = t;
    }
    {
        test<wchar_t> t;
        test<wchar_t> t2;
        t2 = t;
    }
}
