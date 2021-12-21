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

// const_iterator cbegin() const;
// const_iterator cend() const;

#include <regex>
#include <cassert>
#include <cstddef>
#include "test_macros.h"

void
test()
{
    std::match_results<const char*> m;
    const char s[] = "abcdefghijk";
    assert(std::regex_search(s, m, std::regex("cd((e)fg)hi")));

    std::match_results<const char*>::const_iterator i = m.cbegin();
    std::match_results<const char*>::const_iterator e = m.cend();

    assert(static_cast<std::size_t>(e - i) == m.size());
    for (int j = 0; i != e; ++i, ++j)
        assert(*i == m[j]);
}

int main()
{
    test();
}
