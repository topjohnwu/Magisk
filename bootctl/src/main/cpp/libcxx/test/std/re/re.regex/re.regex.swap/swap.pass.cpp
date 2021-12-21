//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// void swap(basic_regex& e);

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    std::regex r1("(a([bc]))");
    std::regex r2;
    r2.swap(r1);
    assert(r1.flags() == std::regex::ECMAScript);
    assert(r1.mark_count() == 0);
    assert(r2.flags() == std::regex::ECMAScript);
    assert(r2.mark_count() == 2);
}
