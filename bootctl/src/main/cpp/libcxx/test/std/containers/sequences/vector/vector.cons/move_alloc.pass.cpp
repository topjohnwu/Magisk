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

// vector(vector&& c, const allocator_type& a);

#include <vector>
#include <cassert>
#include "MoveOnly.h"
#include "test_allocator.h"
#include "min_allocator.h"
#include "asan_testing.h"

int main()
{
    {
        std::vector<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        std::vector<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, test_allocator<MoveOnly> > l2(std::move(l), test_allocator<MoveOnly>(6));
        assert(l2 == lo);
        assert(!l.empty());
        assert(l2.get_allocator() == test_allocator<MoveOnly>(6));
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        std::vector<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        std::vector<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, test_allocator<MoveOnly> > l2(std::move(l), test_allocator<MoveOnly>(5));
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == test_allocator<MoveOnly>(5));
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        std::vector<MoveOnly, other_allocator<MoveOnly> > l(other_allocator<MoveOnly>(5));
        std::vector<MoveOnly, other_allocator<MoveOnly> > lo(other_allocator<MoveOnly>(5));
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, other_allocator<MoveOnly> > l2(std::move(l), other_allocator<MoveOnly>(4));
        assert(l2 == lo);
        assert(!l.empty());
        assert(l2.get_allocator() == other_allocator<MoveOnly>(4));
        assert(is_contiguous_container_asan_correct(l2));
    }
    {
        std::vector<MoveOnly, min_allocator<MoveOnly> > l(min_allocator<MoveOnly>{});
        std::vector<MoveOnly, min_allocator<MoveOnly> > lo(min_allocator<MoveOnly>{});
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        assert(is_contiguous_container_asan_correct(l));
        assert(is_contiguous_container_asan_correct(lo));
        std::vector<MoveOnly, min_allocator<MoveOnly> > l2(std::move(l), min_allocator<MoveOnly>());
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == min_allocator<MoveOnly>());
        assert(is_contiguous_container_asan_correct(l2));
    }
}
