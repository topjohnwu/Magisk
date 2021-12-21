//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// explicit map(const key_compare& comp);

// key_compare key_comp() const;

#include <map>
#include <cassert>

#include "../../../test_compare.h"
#include "min_allocator.h"

int main()
{
    {
    typedef test_compare<std::less<int> > C;
    const std::map<int, double, C> m(C(3));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(3));
    }
#if TEST_STD_VER >= 11
    {
    typedef test_compare<std::less<int> > C;
    const std::map<int, double, C, min_allocator<std::pair<const int, double>>> m(C(3));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(3));
    }
#endif
}
