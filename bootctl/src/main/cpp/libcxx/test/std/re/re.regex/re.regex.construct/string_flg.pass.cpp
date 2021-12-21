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
//    basic_regex(const basic_string<charT, ST, SA>& s,
//                flag_type f = regex_constants::ECMAScript);

#include <regex>
#include <cassert>
#include "test_macros.h"

template <class String>
void
test(const String& p, std::regex_constants::syntax_option_type f, unsigned mc)
{
    std::basic_regex<typename String::value_type> r(p, f);
    assert(r.flags() == f);
    assert(r.mark_count() == mc);
}

int main()
{
    test(std::string("\\(a\\)"), std::regex_constants::basic, 1);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::basic, 1);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::basic, 2);
    test(std::string("(a([bc]))"), std::regex_constants::basic, 0);

    test(std::string("\\(a\\)"), std::regex_constants::extended, 0);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::extended, 0);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::extended, 0);
    test(std::string("(a([bc]))"), std::regex_constants::extended, 2);

    test(std::string("\\(a\\)"), std::regex_constants::ECMAScript, 0);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::ECMAScript, 0);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::ECMAScript, 0);
    test(std::string("(a([bc]))"), std::regex_constants::ECMAScript, 2);

    test(std::string("\\(a\\)"), std::regex_constants::awk, 0);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::awk, 0);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::awk, 0);
    test(std::string("(a([bc]))"), std::regex_constants::awk, 2);

    test(std::string("\\(a\\)"), std::regex_constants::grep, 1);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::grep, 1);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::grep, 2);
    test(std::string("(a([bc]))"), std::regex_constants::grep, 0);

    test(std::string("\\(a\\)"), std::regex_constants::egrep, 0);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::egrep, 0);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::egrep, 0);
    test(std::string("(a([bc]))"), std::regex_constants::egrep, 2);
}
