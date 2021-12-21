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

int main()
{
    using std::regex_constants::awk;

    assert(std::regex_match("\4", std::regex("\\4", awk)));
    assert(std::regex_match("\41", std::regex("\\41", awk)));
    assert(std::regex_match("\141", std::regex("\\141", awk)));
    assert(std::regex_match("\141" "1", std::regex("\\1411", awk)));
}
