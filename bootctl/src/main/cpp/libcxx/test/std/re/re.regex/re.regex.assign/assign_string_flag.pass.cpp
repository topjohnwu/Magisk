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

// template <class string_traits, class A>
//   basic_regex& assign(const basic_string<charT, string_traits, A>& s,
//                       flag_type f = regex_constants::ECMAScript);

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    std::regex r2;
    r2.assign(std::string("(a([bc]))"));
    assert(r2.flags() == std::regex::ECMAScript);
    assert(r2.mark_count() == 2);

    r2.assign(std::string("(a([bc]))"), std::regex::extended);
    assert(r2.flags() == std::regex::extended);
    assert(r2.mark_count() == 2);
}
