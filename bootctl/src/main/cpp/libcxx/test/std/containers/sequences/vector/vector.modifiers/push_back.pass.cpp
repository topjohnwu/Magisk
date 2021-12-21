//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void push_back(const value_type& x);

#include <vector>
#include <cassert>
#include <cstddef>
#include "test_allocator.h"
#include "min_allocator.h"
#include "asan_testing.h"

int main()
{
    {
        std::vector<int> c;
        c.push_back(0);
        assert(c.size() == 1);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(1);
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(2);
        assert(c.size() == 3);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(3);
        assert(c.size() == 4);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(4);
        assert(c.size() == 5);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
    }
    {
        // libc++ needs 15 because it grows by 2x (1 + 2 + 4 + 8).
        // Use 17 for implementations that dynamically allocate a container proxy
        // and grow by 1.5x (1 for proxy + 1 + 2 + 3 + 4 + 6).
        std::vector<int, limited_allocator<int, 17> > c;
        c.push_back(0);
        assert(c.size() == 1);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(1);
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(2);
        assert(c.size() == 3);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(3);
        assert(c.size() == 4);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(4);
        assert(c.size() == 5);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> c;
        c.push_back(0);
        assert(c.size() == 1);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(1);
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(2);
        assert(c.size() == 3);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(3);
        assert(c.size() == 4);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
        c.push_back(4);
        assert(c.size() == 5);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == j);
    }
#endif
}
