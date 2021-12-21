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

// template <class InputIter> vector(InputIter first, InputIter last,
//                                   const allocator_type& a);

#include <vector>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_iterators.h"
#include "min_allocator.h"

template <class C, class Iterator>
void
test(Iterator first, Iterator last, const typename C::allocator_type& a)
{
    C c(first, last, a);
    LIBCPP_ASSERT(c.__invariants());
    assert(c.size() == static_cast<std::size_t>(std::distance(first, last)));
    for (typename C::const_iterator i = c.cbegin(), e = c.cend(); i != e; ++i, ++first)
        assert(*i == *first);
}

int main()
{
    bool a[] = {0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0};
    bool* an = a + sizeof(a)/sizeof(a[0]);
    {
    std::allocator<bool> alloc;
    test<std::vector<bool> >(input_iterator<const bool*>(a), input_iterator<const bool*>(an), alloc);
    test<std::vector<bool> >(forward_iterator<const bool*>(a), forward_iterator<const bool*>(an), alloc);
    test<std::vector<bool> >(bidirectional_iterator<const bool*>(a), bidirectional_iterator<const bool*>(an), alloc);
    test<std::vector<bool> >(random_access_iterator<const bool*>(a), random_access_iterator<const bool*>(an), alloc);
    test<std::vector<bool> >(a, an, alloc);
    }
#if TEST_STD_VER >= 11
    {
    min_allocator<bool> alloc;
    test<std::vector<bool, min_allocator<bool>> >(input_iterator<const bool*>(a), input_iterator<const bool*>(an), alloc);
    test<std::vector<bool, min_allocator<bool>> >(forward_iterator<const bool*>(a), forward_iterator<const bool*>(an), alloc);
    test<std::vector<bool, min_allocator<bool>> >(bidirectional_iterator<const bool*>(a), bidirectional_iterator<const bool*>(an), alloc);
    test<std::vector<bool, min_allocator<bool>> >(random_access_iterator<const bool*>(a), random_access_iterator<const bool*>(an), alloc);
    test<std::vector<bool, min_allocator<bool>> >(a, an, alloc);
    }
#endif
}
