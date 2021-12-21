//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8
// REQUIRES: locale.fr_FR.UTF-8

// <streambuf>

// template <class charT, class traits = char_traits<charT> >
// class basic_streambuf;

// locale pubimbue(const locale& loc);
// locale getloc() const;

#include <streambuf>
#include <cassert>

#include "platform_support.h" // locale name macros

template <class CharT>
struct test
    : public std::basic_streambuf<CharT>
{
    test() {}

    void imbue(const std::locale&)
    {
        assert(this->getloc().name() == LOCALE_en_US_UTF_8);
    }
};

int main()
{
    {
        test<char> t;
        assert(t.getloc().name() == "C");
    }
    std::locale::global(std::locale(LOCALE_en_US_UTF_8));
    {
        test<char> t;
        assert(t.getloc().name() == LOCALE_en_US_UTF_8);
        assert(t.pubimbue(std::locale(LOCALE_fr_FR_UTF_8)).name() ==
               LOCALE_en_US_UTF_8);
        assert(t.getloc().name() == LOCALE_fr_FR_UTF_8);
    }
}
