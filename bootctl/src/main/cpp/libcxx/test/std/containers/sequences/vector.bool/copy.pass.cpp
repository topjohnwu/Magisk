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

// vector(const vector& v);

#include <vector>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

template <class C>
void
test(const C& x)
{
    typename C::size_type s = x.size();
    C c(x);
    LIBCPP_ASSERT(c.__invariants());
    assert(c.size() == s);
    assert(c == x);
}

int main()
{
    {
        bool a[] = {0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0};
        bool* an = a + sizeof(a)/sizeof(a[0]);
        test(std::vector<bool>(a, an));
    }
    {
        std::vector<bool, test_allocator<bool> > v(3, true, test_allocator<bool>(5));
        std::vector<bool, test_allocator<bool> > v2 = v;
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
    }
#if TEST_STD_VER >= 11
    {
        std::vector<bool, other_allocator<bool> > v(3, true, other_allocator<bool>(5));
        std::vector<bool, other_allocator<bool> > v2 = v;
        assert(v2 == v);
        assert(v2.get_allocator() == other_allocator<bool>(-2));
    }
    {
        bool a[] = {0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0};
        bool* an = a + sizeof(a)/sizeof(a[0]);
        test(std::vector<bool, min_allocator<bool>>(a, an));
    }
    {
        std::vector<bool, min_allocator<bool> > v(3, true, min_allocator<bool>());
        std::vector<bool, min_allocator<bool> > v2 = v;
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
    }
#endif
}
