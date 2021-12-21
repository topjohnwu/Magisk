//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class multiset

// multiset(const allocator_type& a);

#include <set>
#include <cassert>

#include "test_allocator.h"

int main()
{
    typedef std::less<int> C;
    typedef test_allocator<int> A;
    std::multiset<int, C, A> m(A(5));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.get_allocator() == A(5));
}
