//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void reserve(size_type n);

#include <vector>
#include <cassert>
#include "test_allocator.h"
#include "min_allocator.h"
#include "asan_testing.h"

int main()
{
    {
        std::vector<int> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        // Add 1 for implementations that dynamically allocate a container proxy.
        std::vector<int, limited_allocator<int, 250 + 1> > v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        assert(is_contiguous_container_asan_correct(v));
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        std::vector<int, min_allocator<int>> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
}
