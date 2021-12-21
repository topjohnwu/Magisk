//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// vector(size_type n, const value_type& x);

#include <vector>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"
#include "asan_testing.h"

template <class C>
void
test(typename C::size_type n, const typename C::value_type& x)
{
    C c(n, x);
    LIBCPP_ASSERT(c.__invariants());
    assert(c.size() == n);
    LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
    for (typename C::const_iterator i = c.cbegin(), e = c.cend(); i != e; ++i)
        assert(*i == x);
}

int main()
{
    test<std::vector<int> >(50, 3);
    // Add 1 for implementations that dynamically allocate a container proxy.
    test<std::vector<int, limited_allocator<int, 50 + 1> > >(50, 5);
#if TEST_STD_VER >= 11
    test<std::vector<int, min_allocator<int>> >(50, 3);
#endif
}
