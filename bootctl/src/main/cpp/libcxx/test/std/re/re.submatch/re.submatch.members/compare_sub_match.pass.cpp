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

// int compare(const sub_match& s) const;

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
        typedef char CharT;
        typedef std::sub_match<const CharT*> SM;
        SM sm = SM();
        SM sm2 = SM();
        assert(sm.compare(sm2) == 0);
        const CharT s[] = {'1', '2', '3', 0};
        sm.first = s;
        sm.second = s + 3;
        sm.matched = true;
        assert(sm.compare(sm2) > 0);
        sm2.first = s;
        sm2.second = s + 3;
        sm2.matched = true;
        assert(sm.compare(sm2) == 0);
    }
    {
        typedef wchar_t CharT;
        typedef std::sub_match<const CharT*> SM;
        SM sm = SM();
        SM sm2 = SM();
        assert(sm.compare(sm2) == 0);
        const CharT s[] = {'1', '2', '3', 0};
        sm.first = s;
        sm.second = s + 3;
        sm.matched = true;
        assert(sm.compare(sm2) > 0);
        sm2.first = s;
        sm2.second = s + 3;
        sm2.matched = true;
        assert(sm.compare(sm2) == 0);
    }
}
