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

// locale_type imbue(locale_type l);

#include <regex>
#include <locale>
#include <cassert>

#include "test_macros.h"
#include "platform_support.h" // locale name macros

int main()
{
    {
        std::regex_traits<char> t;
        std::locale loc = t.imbue(std::locale(LOCALE_en_US_UTF_8));
        assert(loc.name() == "C");
        assert(t.getloc().name() == LOCALE_en_US_UTF_8);
    }
}
