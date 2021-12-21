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

// basic_regex& operator=(initializer_list<charT> il);

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    std::regex r2;
    r2 = {'(', 'a', '(', '[', 'b', 'c', ']', ')', ')'};
    assert(r2.flags() == std::regex::ECMAScript);
    assert(r2.mark_count() == 2);
}
