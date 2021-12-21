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

// size_type max_size() const;

#include <regex>
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test()
{
    std::match_results<const CharT*> m;
    assert(m.max_size() > 0);
}

int main()
{
    test<char>();
    test<wchar_t>();
}
