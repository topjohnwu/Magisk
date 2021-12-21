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

// template <class InputIterator>
//    basic_regex&
//    assign(InputIterator first, InputIterator last,
//           flag_type f = regex_constants::ECMAScript);

#include <regex>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

int main()
{
    typedef input_iterator<std::string::const_iterator> I;
    typedef forward_iterator<std::string::const_iterator> F;
    std::string s4("(a([bc]))");
    std::regex r2;

    r2.assign(I(s4.begin()), I(s4.end()));
    assert(r2.flags() == std::regex::ECMAScript);
    assert(r2.mark_count() == 2);

    r2.assign(I(s4.begin()), I(s4.end()), std::regex::extended);
    assert(r2.flags() == std::regex::extended);
    assert(r2.mark_count() == 2);

    r2.assign(F(s4.begin()), F(s4.end()));
    assert(r2.flags() == std::regex::ECMAScript);
    assert(r2.mark_count() == 2);

    r2.assign(F(s4.begin()), F(s4.end()), std::regex::extended);
    assert(r2.flags() == std::regex::extended);
    assert(r2.mark_count() == 2);
}
