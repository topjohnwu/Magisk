//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator, class Allocator, class charT, class traits>
//     bool
//     regex_search(BidirectionalIterator first, BidirectionalIterator last,
//                  match_results<BidirectionalIterator, Allocator>& m,
//                  const basic_regex<charT, traits>& e,
//                  regex_constants::match_flag_type flags = regex_constants::match_default);

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    // Iterating over /^a/ should yield one instance at the beginning
    // of the text.

    const char *text = "aaa\naa";
    std::regex re("^a");
    std::cregex_iterator it(text, text+6, re);
    std::cregex_iterator end = std::cregex_iterator();

    assert(it->str() == "a");
    assert(it->position(0) == 0);
    assert(it->length(0) == 1);

    ++it;
    assert(it == end);
}
