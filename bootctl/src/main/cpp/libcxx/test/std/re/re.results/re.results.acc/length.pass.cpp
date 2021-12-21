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

// difference_type length(size_type sub = 0) const;

#include <regex>
#include <cassert>
#include "test_macros.h"

void
test()
{
    std::match_results<const char*> m;
    const char s[] = "abcdefghijk";
    assert(std::regex_search(s, m, std::regex("cd((e)fg)hi")));
    assert(m.length() == m[0].length());
    assert(m.length(0) == m[0].length());
    assert(m.length(1) == m[1].length());
    assert(m.length(2) == m[2].length());
    assert(m.length(3) == m[3].length());
    assert(m.length(4) == m[4].length());
}

int main()
{
    test();
}
