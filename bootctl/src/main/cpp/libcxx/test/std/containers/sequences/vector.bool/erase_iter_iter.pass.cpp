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

// iterator erase(const_iterator first, const_iterator last);

#include <vector>
#include <cassert>

#include "min_allocator.h"

int main()
{
    bool a1[] = {1, 0, 1};
    {
        std::vector<bool> l1(a1, a1+3);
        std::vector<bool>::iterator i = l1.erase(l1.cbegin(), l1.cbegin());
        assert(l1.size() == 3);
        assert(distance(l1.cbegin(), l1.cend()) == 3);
        assert(i == l1.begin());
    }
    {
        std::vector<bool> l1(a1, a1+3);
        std::vector<bool>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin()));
        assert(l1.size() == 2);
        assert(distance(l1.cbegin(), l1.cend()) == 2);
        assert(i == l1.begin());
        assert(l1 == std::vector<bool>(a1+1, a1+3));
    }
    {
        std::vector<bool> l1(a1, a1+3);
        std::vector<bool>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 2));
        assert(l1.size() == 1);
        assert(distance(l1.cbegin(), l1.cend()) == 1);
        assert(i == l1.begin());
        assert(l1 == std::vector<bool>(a1+2, a1+3));
    }
    {
        std::vector<bool> l1(a1, a1+3);
        std::vector<bool>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 3));
        assert(l1.size() == 0);
        assert(distance(l1.cbegin(), l1.cend()) == 0);
        assert(i == l1.begin());
    }
#if TEST_STD_VER >= 11
    {
        std::vector<bool, min_allocator<bool>> l1(a1, a1+3);
        std::vector<bool, min_allocator<bool>>::iterator i = l1.erase(l1.cbegin(), l1.cbegin());
        assert(l1.size() == 3);
        assert(distance(l1.cbegin(), l1.cend()) == 3);
        assert(i == l1.begin());
    }
    {
        std::vector<bool, min_allocator<bool>> l1(a1, a1+3);
        std::vector<bool, min_allocator<bool>>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin()));
        assert(l1.size() == 2);
        assert(distance(l1.cbegin(), l1.cend()) == 2);
        assert(i == l1.begin());
        assert((l1 == std::vector<bool, min_allocator<bool>>(a1+1, a1+3)));
    }
    {
        std::vector<bool, min_allocator<bool>> l1(a1, a1+3);
        std::vector<bool, min_allocator<bool>>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 2));
        assert(l1.size() == 1);
        assert(distance(l1.cbegin(), l1.cend()) == 1);
        assert(i == l1.begin());
        assert((l1 == std::vector<bool, min_allocator<bool>>(a1+2, a1+3)));
    }
    {
        std::vector<bool, min_allocator<bool>> l1(a1, a1+3);
        std::vector<bool, min_allocator<bool>>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 3));
        assert(l1.size() == 0);
        assert(distance(l1.cbegin(), l1.cend()) == 0);
        assert(i == l1.begin());
    }
#endif
}
