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

// difference_type length() const;

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
        typedef char CharT;
        typedef std::sub_match<const CharT*> SM;
        SM sm = SM();
        assert(sm.length() == 0);
        const CharT s[] = {'1', '2', '3', 0};
        sm.first = s;
        sm.second = s + 3;
        sm.matched = true;
        assert(sm.length() == 3);
    }
    {
        typedef wchar_t CharT;
        typedef std::sub_match<const CharT*> SM;
        SM sm = SM();
        assert(sm.length() == 0);
        const CharT s[] = {'1', '2', '3', 0};
        sm.first = s;
        sm.second = s + 3;
        sm.matched = true;
        assert(sm.length() == 3);
    }
}
