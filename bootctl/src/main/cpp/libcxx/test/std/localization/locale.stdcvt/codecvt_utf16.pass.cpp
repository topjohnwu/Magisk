//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <codecvt>

// template <class Elem, unsigned long Maxcode = 0x10ffff,
//           codecvt_mode Mode = (codecvt_mode)0>
// class codecvt_utf16
//     : public codecvt<Elem, char, mbstate_t>
// {
//     // unspecified
// };

// Not a portable test

#include <codecvt>
#include <cstdlib>
#include <cassert>

#include "count_new.hpp"

int main()
{
    assert(globalMemCounter.checkOutstandingNewEq(0));
    {
        typedef std::codecvt_utf16<wchar_t> C;
        C c;
        assert(globalMemCounter.checkOutstandingNewEq(0));
    }
    {
        typedef std::codecvt_utf16<wchar_t> C;
        std::locale loc(std::locale::classic(), new C);
        assert(globalMemCounter.checkOutstandingNewNotEq(0));
    }
    assert(globalMemCounter.checkOutstandingNewEq(0));
}
