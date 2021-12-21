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

// void swap(basic_streambuf& rhs);

#include <streambuf>
#include <cassert>

#include "platform_support.h" // locale name macros

template <class CharT>
struct test
    : public std::basic_streambuf<CharT>
{
    typedef std::basic_streambuf<CharT> base;
    test() {}

    void swap(test& t)
    {
        test old_this(*this);
        test old_that(t);
        base::swap(t);
        assert(this->eback() == old_that.eback());
        assert(this->gptr()  == old_that.gptr());
        assert(this->egptr() == old_that.egptr());
        assert(this->pbase() == old_that.pbase());
        assert(this->pptr()  == old_that.pptr());
        assert(this->epptr() == old_that.epptr());
        assert(this->getloc() == old_that.getloc());

        assert(t.eback() == old_this.eback());
        assert(t.gptr()  == old_this.gptr());
        assert(t.egptr() == old_this.egptr());
        assert(t.pbase() == old_this.pbase());
        assert(t.pptr()  == old_this.pptr());
        assert(t.epptr() == old_this.epptr());
        assert(t.getloc() == old_this.getloc());
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
        t2.swap(t);
    }
    {
        test<wchar_t> t;
        test<wchar_t> t2;
        t2.swap(t);
    }
    {
        char g1, g2, g3, p1, p3;
        test<char> t;
        t.setg(&g1, &g2, &g3);
        t.setp(&p1, &p3);
        test<char> t2;
        t2.swap(t);
    }
    {
        wchar_t g1, g2, g3, p1, p3;
        test<wchar_t> t;
        t.setg(&g1, &g2, &g3);
        t.setp(&p1, &p3);
        test<wchar_t> t2;
        t2.swap(t);
    }
    std::locale::global(std::locale(LOCALE_en_US_UTF_8));
    {
        test<char> t;
        test<char> t2;
        t2.swap(t);
    }
    {
        test<wchar_t> t;
        test<wchar_t> t2;
        t2.swap(t);
    }
}
