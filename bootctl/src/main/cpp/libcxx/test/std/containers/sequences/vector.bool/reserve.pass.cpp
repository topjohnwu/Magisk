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

// void reserve(size_type n);

#include <vector>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        std::vector<bool> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
    }
    {
        std::vector<bool> v(100);
        assert(v.capacity() >= 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() >= 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() >= 150);
    }
#if TEST_STD_VER >= 11
    {
        std::vector<bool, min_allocator<bool>> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
    }
    {
        std::vector<bool, min_allocator<bool>> v(100);
        assert(v.capacity() >= 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() >= 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() >= 150);
    }
#endif
}
