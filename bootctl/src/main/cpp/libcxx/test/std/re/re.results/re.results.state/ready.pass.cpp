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

// bool ready() const;

#include <regex>
#include <cassert>
#include "test_macros.h"

void
test1()
{
    std::match_results<const char*> m;
    const char s[] = "abcdefghijk";
    assert(m.ready() == false);
    std::regex_search(s, m, std::regex("cd((e)fg)hi"));
    assert(m.ready() == true);
}

void
test2()
{
    std::match_results<const char*> m;
    const char s[] = "abcdefghijk";
    assert(m.ready() == false);
    std::regex_search(s, m, std::regex("z"));
    assert(m.ready() == true);
}

int main()
{
    test1();
    test2();
}
