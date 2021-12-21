//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// template <class BidirectionalIterator, class Allocator>
//    void swap(match_results<BidirectionalIterator, Allocator>& m1,
//              match_results<BidirectionalIterator, Allocator>& m2);

#include <regex>
#include <cassert>
#include "test_macros.h"

void
test()
{
    std::match_results<const char*> m1;
    const char s[] = "abcdefghijk";
    assert(std::regex_search(s, m1, std::regex("cd((e)fg)hi")));
    std::match_results<const char*> m2;

    std::match_results<const char*> m1_save = m1;
    std::match_results<const char*> m2_save = m2;

    swap(m1, m2);

    assert(m1 == m2_save);
    assert(m2 == m1_save);
}

int main()
{
    test();
}
