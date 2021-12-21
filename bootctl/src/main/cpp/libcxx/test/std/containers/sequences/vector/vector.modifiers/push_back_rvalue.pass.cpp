//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <vector>

// void push_back(value_type&& x);

#include <vector>
#include <cassert>
#include <cstddef>
#include "MoveOnly.h"
#include "test_allocator.h"
#include "min_allocator.h"
#include "asan_testing.h"

int main()
{
    {
        std::vector<MoveOnly> c;
        c.push_back(MoveOnly(0));
        assert(c.size() == 1);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(1));
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(2));
        assert(c.size() == 3);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(3));
        assert(c.size() == 4);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(4));
        assert(c.size() == 5);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
    }
    {
        // libc++ needs 15 because it grows by 2x (1 + 2 + 4 + 8).
        // Use 17 for implementations that dynamically allocate a container proxy
        // and grow by 1.5x (1 for proxy + 1 + 2 + 3 + 4 + 6).
        std::vector<MoveOnly, limited_allocator<MoveOnly, 17> > c;
        c.push_back(MoveOnly(0));
        assert(c.size() == 1);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(1));
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(2));
        assert(c.size() == 3);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(3));
        assert(c.size() == 4);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(4));
        assert(c.size() == 5);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
    }
    {
        std::vector<MoveOnly, min_allocator<MoveOnly>> c;
        c.push_back(MoveOnly(0));
        assert(c.size() == 1);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(1));
        assert(c.size() == 2);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(2));
        assert(c.size() == 3);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(3));
        assert(c.size() == 4);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
        c.push_back(MoveOnly(4));
        assert(c.size() == 5);
        assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; static_cast<std::size_t>(j) < c.size(); ++j)
            assert(c[j] == MoveOnly(j));
    }
}
