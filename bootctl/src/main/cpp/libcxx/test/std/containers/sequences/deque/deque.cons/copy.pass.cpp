//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// deque(const deque&);

#include <deque>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

template <class C>
void
test(const C& x)
{
    C c(x);
    assert(c == x);
}

int main()
{
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab)/sizeof(ab[0]);
        test(std::deque<int>(ab, an));
    }
    {
        std::deque<int, test_allocator<int> > v(3, 2, test_allocator<int>(5));
        std::deque<int, test_allocator<int> > v2 = v;
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
    }
#if TEST_STD_VER >= 11
    {
        std::deque<int, other_allocator<int> > v(3, 2, other_allocator<int>(5));
        std::deque<int, other_allocator<int> > v2 = v;
        assert(v2 == v);
        assert(v2.get_allocator() == other_allocator<int>(-2));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab)/sizeof(ab[0]);
        test(std::deque<int, min_allocator<int>>(ab, an));
    }
    {
        std::deque<int, min_allocator<int> > v(3, 2, min_allocator<int>());
        std::deque<int, min_allocator<int> > v2 = v;
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
    }
#endif
}
