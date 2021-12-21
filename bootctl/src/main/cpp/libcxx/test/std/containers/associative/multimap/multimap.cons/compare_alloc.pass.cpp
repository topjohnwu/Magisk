//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class multimap

// multimap(const key_compare& comp, const allocator_type& a);

#include <map>
#include <cassert>

#include "../../../test_compare.h"
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
    typedef test_compare<std::less<int> > C;
    typedef test_allocator<std::pair<const int, double> > A;
    std::multimap<int, double, C, A> m(C(4), A(5));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(4));
    assert(m.get_allocator() == A(5));
    }
#if TEST_STD_VER >= 11
    {
    typedef test_compare<std::less<int> > C;
    typedef min_allocator<std::pair<const int, double> > A;
    std::multimap<int, double, C, A> m(C(4), A());
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(4));
    assert(m.get_allocator() == A());
    }
    {
    typedef test_compare<std::less<int> > C;
    typedef explicit_allocator<std::pair<const int, double> > A;
    std::multimap<int, double, C, A> m(C(4), A{});
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(4));
    assert(m.get_allocator() == A{});
    }
#endif
}
