//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// iterator insert(const_iterator position, size_type n, const value_type& x);

#include <vector>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"
#include "asan_testing.h"

int main()
{
    {
        std::vector<int> v(100);
        std::vector<int>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        std::vector<int> v(100);
        while(v.size() < v.capacity()) v.push_back(0); // force reallocation
        size_t sz = v.size();
        std::vector<int>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == sz + 5);
        assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
    {
        std::vector<int> v(100);
        v.reserve(128); // force no reallocation
        size_t sz = v.size();
        std::vector<int>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == sz + 5);
        assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        std::size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
    {
        std::vector<int, limited_allocator<int, 300> > v(100);
        std::vector<int, limited_allocator<int, 300> >::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < 105; ++j)
            assert(v[j] == 0);
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> v(100);
        std::vector<int, min_allocator<int>>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        std::vector<int, min_allocator<int>> v(100);
        std::vector<int, min_allocator<int>>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < 105; ++j)
            assert(v[j] == 0);
    }
#endif
}
