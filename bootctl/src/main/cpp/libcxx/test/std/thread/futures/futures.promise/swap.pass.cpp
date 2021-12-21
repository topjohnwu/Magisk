//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: c++98, c++03

// <future>

// class promise<R>

// void swap(promise& other);

// template <class R> void swap(promise<R>& x, promise<R>& y);

#include <future>
#include <cassert>

#include "test_allocator.h"

int main()
{
    assert(test_alloc_base::alloc_count == 0);
    {
        std::promise<int> p0(std::allocator_arg, test_allocator<int>());
        std::promise<int> p(std::allocator_arg, test_allocator<int>());
        assert(test_alloc_base::alloc_count == 2);
        p.swap(p0);
        assert(test_alloc_base::alloc_count == 2);
        std::future<int> f = p.get_future();
        assert(test_alloc_base::alloc_count == 2);
        assert(f.valid());
        f = p0.get_future();
        assert(f.valid());
        assert(test_alloc_base::alloc_count == 2);
    }
    assert(test_alloc_base::alloc_count == 0);
    {
        std::promise<int> p0(std::allocator_arg, test_allocator<int>());
        std::promise<int> p(std::allocator_arg, test_allocator<int>());
        assert(test_alloc_base::alloc_count == 2);
        swap(p, p0);
        assert(test_alloc_base::alloc_count == 2);
        std::future<int> f = p.get_future();
        assert(test_alloc_base::alloc_count == 2);
        assert(f.valid());
        f = p0.get_future();
        assert(f.valid());
        assert(test_alloc_base::alloc_count == 2);
    }
    assert(test_alloc_base::alloc_count == 0);
    {
        std::promise<int> p0(std::allocator_arg, test_allocator<int>());
        std::promise<int> p;
        assert(test_alloc_base::alloc_count == 1);
        p.swap(p0);
        assert(test_alloc_base::alloc_count == 1);
        std::future<int> f = p.get_future();
        assert(test_alloc_base::alloc_count == 1);
        assert(f.valid());
        f = p0.get_future();
        assert(f.valid());
        assert(test_alloc_base::alloc_count == 1);
    }
    assert(test_alloc_base::alloc_count == 0);
    {
        std::promise<int> p0(std::allocator_arg, test_allocator<int>());
        std::promise<int> p;
        assert(test_alloc_base::alloc_count == 1);
        swap(p, p0);
        assert(test_alloc_base::alloc_count == 1);
        std::future<int> f = p.get_future();
        assert(test_alloc_base::alloc_count == 1);
        assert(f.valid());
        f = p0.get_future();
        assert(f.valid());
        assert(test_alloc_base::alloc_count == 1);
    }
    assert(test_alloc_base::alloc_count == 0);
}
