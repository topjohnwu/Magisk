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
//    bool
//    operator==(const match_results<BidirectionalIterator, Allocator>& m1,
//               const match_results<BidirectionalIterator, Allocator>& m2);

// template <class BidirectionalIterator, class Allocator>
//    bool
//    operator!=(const match_results<BidirectionalIterator, Allocator>& m1,
//               const match_results<BidirectionalIterator, Allocator>& m2);

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

    assert(m1 == m1);
    assert(m1 != m2);

    m2 = m1;

    assert(m1 == m2);
}

int main()
{
    test();
}
