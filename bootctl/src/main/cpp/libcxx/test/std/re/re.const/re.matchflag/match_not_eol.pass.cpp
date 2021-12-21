// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// match_not_eol:
//     The last character in the sequence [first,last) shall be treated as
//     though it is not at the end of a line, so the character "$" in
//     the regular expression shall not match [last,last).

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
    std::string target = "foo";
    std::regex re("foo$");
    assert( std::regex_match(target, re));
    assert(!std::regex_match(target, re, std::regex_constants::match_not_eol));
    }

    {
    std::string target = "foo";
    std::regex re("foo");
    assert( std::regex_match(target, re));
    assert( std::regex_match(target, re, std::regex_constants::match_not_eol));
    }

    {
    std::string target = "refoo";
    std::regex re("foo$");
    assert( std::regex_search(target, re));
    assert(!std::regex_search(target, re, std::regex_constants::match_not_eol));
    }

    {
    std::string target = "refoo";
    std::regex re("foo");
    assert( std::regex_search(target, re));
    assert( std::regex_search(target, re, std::regex_constants::match_not_eol));
    }
}
