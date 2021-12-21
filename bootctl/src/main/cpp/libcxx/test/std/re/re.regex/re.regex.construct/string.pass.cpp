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

// template <class ST, class SA>
//    basic_regex(const basic_string<charT, ST, SA>& s);

#include <regex>
#include <cassert>
#include "test_macros.h"

template <class String>
void
test(const String& p, unsigned mc)
{
    std::basic_regex<typename String::value_type> r(p);
    assert(r.flags() == std::regex_constants::ECMAScript);
    assert(r.mark_count() == mc);
}

int main()
{
    test(std::string("\\(a\\)"), 0);
    test(std::string("\\(a[bc]\\)"), 0);
    test(std::string("\\(a\\([bc]\\)\\)"), 0);
    test(std::string("(a([bc]))"), 2);
}
