//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator> class sub_match;

// constexpr sub_match();

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
        typedef char CharT;
        typedef std::sub_match<const CharT*> SM;
        SM sm;
        assert(sm.matched == false);
    }
    {
        typedef wchar_t CharT;
        typedef std::sub_match<const CharT*> SM;
        SM sm;
        assert(sm.matched == false);
    }
}
