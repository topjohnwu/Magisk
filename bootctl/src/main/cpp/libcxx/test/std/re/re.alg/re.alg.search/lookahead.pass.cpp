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

// https://bugs.llvm.org/show_bug.cgi?id=11118

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    assert(!std::regex_search("ab", std::regex("(?=^)b")));
    assert(!std::regex_search("ab", std::regex("a(?=^)b")));
}
