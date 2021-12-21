//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// deque(const deque& c, const allocator_type& a);

#include <deque>
#include <cassert>

#include "test_allocator.h"
#include "min_allocator.h"

template <class C>
void
test(const C& x, const typename C::allocator_type& a)
{
    C c(x, a);
    assert(c == x);
    assert(c.get_allocator() == a);
}

int main()
{
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab)/sizeof(ab[0]);
        test(std::deque<int, test_allocator<int> >(ab, an, test_allocator<int>(3)),
                                                           test_allocator<int>(4));
    }
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab)/sizeof(ab[0]);
        test(std::deque<int, other_allocator<int> >(ab, an, other_allocator<int>(3)),
                                                            other_allocator<int>(4));
    }
#if TEST_STD_VER >= 11
    {
        int ab[] = {3, 4, 2, 8, 0, 1, 44, 34, 45, 96, 80, 1, 13, 31, 45};
        int* an = ab + sizeof(ab)/sizeof(ab[0]);
        test(std::deque<int, min_allocator<int> >(ab, an, min_allocator<int>()),
                                                          min_allocator<int>());
    }
#endif
}
