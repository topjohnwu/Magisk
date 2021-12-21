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

// match_results(const Allocator& a = Allocator());

#include <regex>
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test()
{
    std::match_results<const CharT*> m;
    assert(m.size() == 0);
    assert(m.str() == std::basic_string<CharT>());
    assert(m.get_allocator() == std::allocator<std::sub_match<const CharT*> >());
}

int main()
{
    test<char>();
    test<wchar_t>();
}
