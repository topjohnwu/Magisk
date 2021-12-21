//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// vector& operator=(const vector& c);

#include <vector>
#include <cassert>
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
        std::vector<bool, test_allocator<bool> > l(3, true, test_allocator<bool>(5));
        std::vector<bool, test_allocator<bool> > l2(l, test_allocator<bool>(3));
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == test_allocator<bool>(3));
    }
    {
        std::vector<bool, other_allocator<bool> > l(3, true, other_allocator<bool>(5));
        std::vector<bool, other_allocator<bool> > l2(l, other_allocator<bool>(3));
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == other_allocator<bool>(5));
    }
#if TEST_STD_VER >= 11
    {
        std::vector<bool, min_allocator<bool> > l(3, true, min_allocator<bool>());
        std::vector<bool, min_allocator<bool> > l2(l, min_allocator<bool>());
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == min_allocator<bool>());
    }
#endif
}
