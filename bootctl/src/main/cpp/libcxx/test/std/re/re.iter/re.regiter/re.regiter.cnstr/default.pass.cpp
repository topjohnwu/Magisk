//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// class regex_iterator<BidirectionalIterator, charT, traits>

// regex_iterator();

#include <regex>
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test()
{
    typedef std::regex_iterator<const CharT*> I;
    I i1;
    assert(i1 == I());
}

int main()
{
    test<char>();
    test<wchar_t>();
}
