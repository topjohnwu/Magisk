// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <regex>

// template <class charT> struct regex_traits;

// regex_traits();

#include <regex>
#include <cassert>

#include "test_macros.h"
#include "platform_support.h" // locale name macros

int main()
{
    {
        std::regex_traits<char> t1;
        assert(t1.getloc().name() == "C");
        std::regex_traits<wchar_t> t2;
        assert(t2.getloc().name() == "C");
    }
    {
        std::locale::global(std::locale(LOCALE_en_US_UTF_8));
        std::regex_traits<char> t1;
        assert(t1.getloc().name() == LOCALE_en_US_UTF_8);
        std::regex_traits<wchar_t> t2;
        assert(t2.getloc().name() == LOCALE_en_US_UTF_8);
    }
}
