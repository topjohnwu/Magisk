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

// basic_regex(const charT* p, size_t len);

#include <regex>
#include <cassert>

template <class CharT>
void
test(const CharT* p, std::size_t len, unsigned mc)
{
    std::basic_regex<CharT> r(p, len);
    assert(r.flags() == std::regex_constants::ECMAScript);
    assert(r.mark_count() == mc);
}

int main()
{
    test("\\(a\\)", 5, 0);
    test("\\(a[bc]\\)", 9, 0);
    test("\\(a\\([bc]\\)\\)", 13, 0);
    test("(a([bc]))", 9, 2);

    test("(\0)(b)(c)(d)", 12, 4);
    test("(\0)(b)(c)(d)", 9, 3);
    test("(\0)(b)(c)(d)", 3, 1);
    test("(\0)(b)(c)(d)", 0, 0);
}
