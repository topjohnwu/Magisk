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

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// locale_type imbue(locale_type loc);

#include <regex>
#include <locale>
#include <cassert>

#include "test_macros.h"
#include "platform_support.h" // locale name macros

int main()
{
    std::regex r;
    std::locale loc = r.imbue(std::locale(LOCALE_en_US_UTF_8));
    assert(loc.name() == "C");
    assert(r.getloc().name() == LOCALE_en_US_UTF_8);
    loc = r.imbue(std::locale("C"));
    assert(loc.name() == LOCALE_en_US_UTF_8);
    assert(r.getloc().name() == "C");
}
