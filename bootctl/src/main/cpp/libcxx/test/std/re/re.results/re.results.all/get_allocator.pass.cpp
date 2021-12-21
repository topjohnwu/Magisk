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

// allocator_type get_allocator() const;

#include <regex>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"

template <class CharT, class Allocator>
void
test(const Allocator& a)
{
    std::match_results<const CharT*, Allocator> m(a);
    assert(m.size() == 0);
    assert(m.str() == std::basic_string<CharT>());
    assert(m.get_allocator() == a);
}

int main()
{
    test<char>(test_allocator<std::sub_match<const char*> >(3));
    test<wchar_t>(test_allocator<std::sub_match<const wchar_t*> >(3));
}
