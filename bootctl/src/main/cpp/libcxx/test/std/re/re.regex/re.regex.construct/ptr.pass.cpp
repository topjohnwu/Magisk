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

// basic_regex(const charT* p);

#include <regex>
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test(const CharT* p, unsigned mc)
{
    std::basic_regex<CharT> r(p);
    assert(r.flags() == std::regex_constants::ECMAScript);
    assert(r.mark_count() == mc);
}

int main()
{
    test("\\(a\\)", 0);
    test("\\(a[bc]\\)", 0);
    test("\\(a\\([bc]\\)\\)", 0);
    test("(a([bc]))", 2);
}
