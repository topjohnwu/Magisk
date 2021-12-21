//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions
// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// template <class ST, class SA>
//    basic_regex(const basic_string<charT, ST, SA>& s);

#include <regex>
#include <cassert>
#include "test_macros.h"

static bool error_badrepeat_thrown(const char *pat)
{
    bool result = false;
    try {
        std::regex re(pat);
    } catch (const std::regex_error &ex) {
        result = (ex.code() == std::regex_constants::error_badrepeat);
    }
    return result;
}

int main()
{
    assert(error_badrepeat_thrown("?a"));
    assert(error_badrepeat_thrown("*a"));
    assert(error_badrepeat_thrown("+a"));
    assert(error_badrepeat_thrown("{a"));

    assert(error_badrepeat_thrown("?(a+)"));
    assert(error_badrepeat_thrown("*(a+)"));
    assert(error_badrepeat_thrown("+(a+)"));
    assert(error_badrepeat_thrown("{(a+)"));
}
