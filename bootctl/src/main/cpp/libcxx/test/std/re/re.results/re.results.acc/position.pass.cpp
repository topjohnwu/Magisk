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

// difference_type position(size_type sub = 0) const;

#include <regex>
#include <cassert>
#include "test_macros.h"

void
test()
{
    std::match_results<const char*> m;
    const char s[] = "abcdefghijk";
    assert(std::regex_search(s, m, std::regex("cd((e)fg)hi")));
    assert(m.position() == std::distance(s, m[0].first));
    assert(m.position(0) == std::distance(s, m[0].first));
    assert(m.position(1) == std::distance(s, m[1].first));
    assert(m.position(2) == std::distance(s, m[2].first));
    assert(m.position(3) == std::distance(s, m[3].first));
    assert(m.position(4) == std::distance(s, m[4].first));
}

int main()
{
    test();
}
