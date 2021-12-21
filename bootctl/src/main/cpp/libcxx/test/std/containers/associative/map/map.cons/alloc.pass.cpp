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

// explicit map(const allocator_type& a);

#include <map>
#include <cassert>

#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::less<int> C;
    typedef test_allocator<std::pair<const int, double> > A;
    std::map<int, double, C, A> m(A(5));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.get_allocator() == A(5));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::less<int> C;
    typedef min_allocator<std::pair<const int, double> > A;
    std::map<int, double, C, A> m(A{});
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.get_allocator() == A());
    }
    {
    typedef std::less<int> C;
    typedef explicit_allocator<std::pair<const int, double> > A;
    std::map<int, double, C, A> m(A{});
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.get_allocator() == A());
    }
#endif
}
