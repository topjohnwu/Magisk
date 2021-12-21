//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>
// vector<bool>

// vector(size_type n, const value_type& x, const allocator_type& a);

#include <vector>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class C>
void
test(typename C::size_type n, const typename C::value_type& x,
     const typename C::allocator_type& a)
{
    C c(n, x, a);
    LIBCPP_ASSERT(c.__invariants());
    assert(a == c.get_allocator());
    assert(c.size() == n);
    for (typename C::const_iterator i = c.cbegin(), e = c.cend(); i != e; ++i)
        assert(*i == x);
}

int main()
{
    test<std::vector<bool> >(50, true, std::allocator<bool>());
#if TEST_STD_VER >= 11
    test<std::vector<bool, min_allocator<bool>> >(50, true, min_allocator<bool>());
#endif
}
