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

// match_not_bol:
//     The first character in the sequence [first,last) shall be treated as
//     though it is not at the beginning of a line, so the character ^ in the
//     regular expression shall not match [first,first).

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
    std::string target = "foo";
    std::regex re("^foo");
    assert( std::regex_match(target, re));
    assert(!std::regex_match(target, re, std::regex_constants::match_not_bol));
    }

    {
    std::string target = "foo";
    std::regex re("foo");
    assert( std::regex_match(target, re));
    assert( std::regex_match(target, re, std::regex_constants::match_not_bol));
    }

    {
    std::string target = "fooby";
    std::regex re("^foo");
    assert( std::regex_search(target, re));
    assert(!std::regex_search(target, re, std::regex_constants::match_not_bol));
    }

    {
    std::string target = "fooby";
    std::regex re("foo");
    assert( std::regex_search(target, re));
    assert( std::regex_search(target, re, std::regex_constants::match_not_bol));
    }
}
