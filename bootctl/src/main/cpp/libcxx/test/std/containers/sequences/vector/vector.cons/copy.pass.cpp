//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// vector(const vector& v);

#include <vector>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"
#include "asan_testing.h"

template <class C>
void
test(const C& x)
{
    typename C::size_type s = x.size();
    C c(x);
    LIBCPP_ASSERT(c.__invariants());
    assert(c.size() == s);
    assert(c == x);
    LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
}

int main()
{
    {
        int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 1, 0};
        int* an = a + sizeof(a)/sizeof(a[0]);
        test(std::vector<int>(a, an));
    }
    {
        std::vector<int, test_allocator<int> > v(3, 2, test_allocator<int>(5));
        std::vector<int, test_allocator<int> > v2 = v;
        assert(is_contiguous_container_asan_correct(v));
        assert(is_contiguous_container_asan_correct(v2));
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
        assert(is_contiguous_container_asan_correct(v));
        assert(is_contiguous_container_asan_correct(v2));
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, other_allocator<int> > v(3, 2, other_allocator<int>(5));
        std::vector<int, other_allocator<int> > v2 = v;
        assert(is_contiguous_container_asan_correct(v));
        assert(is_contiguous_container_asan_correct(v2));
        assert(v2 == v);
        assert(v2.get_allocator() == other_allocator<int>(-2));
        assert(is_contiguous_container_asan_correct(v));
        assert(is_contiguous_container_asan_correct(v2));
    }
    {
        int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 1, 0};
        int* an = a + sizeof(a)/sizeof(a[0]);
        test(std::vector<int, min_allocator<int>>(a, an));
    }
    {
        std::vector<int, min_allocator<int> > v(3, 2, min_allocator<int>());
        std::vector<int, min_allocator<int> > v2 = v;
        assert(is_contiguous_container_asan_correct(v));
        assert(is_contiguous_container_asan_correct(v2));
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
        assert(is_contiguous_container_asan_correct(v));
        assert(is_contiguous_container_asan_correct(v2));
    }
#endif
}
