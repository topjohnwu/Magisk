//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// basic_regex(initializer_list<charT> il,
//             flag_type f = regex_constants::ECMAScript);

#include <regex>
#include <cassert>
#include "test_macros.h"


void
test(std::initializer_list<char> il, std::regex_constants::syntax_option_type f, unsigned mc)
{
    std::basic_regex<char> r(il, f);
    assert(r.flags() == f);
    assert(r.mark_count() == mc);
}


int main()
{
    std::string s1("\\(a\\)");
    std::string s2("\\(a[bc]\\)");
    std::string s3("\\(a\\([bc]\\)\\)");
    std::string s4("(a([bc]))");

    test({'\\', '(', 'a', '\\', ')'}, std::regex_constants::basic, 1);
    test({'\\', '(', 'a', '[', 'b', 'c', ']', '\\', ')'}, std::regex_constants::basic, 1);
    test({'\\', '(', 'a', '\\', '(', '[', 'b', 'c', ']', '\\', ')', '\\', ')'}, std::regex_constants::basic, 2);
    test({'(', 'a', '(', '[', 'b', 'c', ']', ')', ')'}, std::regex_constants::basic, 0);

    test({'\\', '(', 'a', '\\', ')'}, std::regex_constants::extended, 0);
    test({'\\', '(', 'a', '[', 'b', 'c', ']', '\\', ')'}, std::regex_constants::extended, 0);
    test({'\\', '(', 'a', '\\', '(', '[', 'b', 'c', ']', '\\', ')', '\\', ')'}, std::regex_constants::extended, 0);
    test({'(', 'a', '(', '[', 'b', 'c', ']', ')', ')'}, std::regex_constants::extended, 2);

    test({'\\', '(', 'a', '\\', ')'}, std::regex_constants::ECMAScript, 0);
    test({'\\', '(', 'a', '[', 'b', 'c', ']', '\\', ')'}, std::regex_constants::ECMAScript, 0);
    test({'\\', '(', 'a', '\\', '(', '[', 'b', 'c', ']', '\\', ')', '\\', ')'}, std::regex_constants::ECMAScript, 0);
    test({'(', 'a', '(', '[', 'b', 'c', ']', ')', ')'}, std::regex_constants::ECMAScript, 2);

    test({'\\', '(', 'a', '\\', ')'}, std::regex_constants::awk, 0);
    test({'\\', '(', 'a', '[', 'b', 'c', ']', '\\', ')'}, std::regex_constants::awk, 0);
    test({'\\', '(', 'a', '\\', '(', '[', 'b', 'c', ']', '\\', ')', '\\', ')'}, std::regex_constants::awk, 0);
    test({'(', 'a', '(', '[', 'b', 'c', ']', ')', ')'}, std::regex_constants::awk, 2);

    test({'\\', '(', 'a', '\\', ')'}, std::regex_constants::grep, 1);
    test({'\\', '(', 'a', '[', 'b', 'c', ']', '\\', ')'}, std::regex_constants::grep, 1);
    test({'\\', '(', 'a', '\\', '(', '[', 'b', 'c', ']', '\\', ')', '\\', ')'}, std::regex_constants::grep, 2);
    test({'(', 'a', '(', '[', 'b', 'c', ']', ')', ')'}, std::regex_constants::grep, 0);

    test({'\\', '(', 'a', '\\', ')'}, std::regex_constants::egrep, 0);
    test({'\\', '(', 'a', '[', 'b', 'c', ']', '\\', ')'}, std::regex_constants::egrep, 0);
    test({'\\', '(', 'a', '\\', '(', '[', 'b', 'c', ']', '\\', ')', '\\', ')'}, std::regex_constants::egrep, 0);
    test({'(', 'a', '(', '[', 'b', 'c', ']', ')', ')'}, std::regex_constants::egrep, 2);
}
