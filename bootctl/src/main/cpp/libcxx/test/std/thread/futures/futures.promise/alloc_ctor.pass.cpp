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

// template <class Allocator>
//   promise(allocator_arg_t, const Allocator& a);

#include <future>
#include <cassert>

#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    assert(test_alloc_base::alloc_count == 0);
    {
        std::promise<int> p(std::allocator_arg, test_allocator<int>(42));
        assert(test_alloc_base::alloc_count == 1);
        std::future<int> f = p.get_future();
        assert(test_alloc_base::alloc_count == 1);
        assert(f.valid());
    }
    assert(test_alloc_base::alloc_count == 0);
    {
        std::promise<int&> p(std::allocator_arg, test_allocator<int>(42));
        assert(test_alloc_base::alloc_count == 1);
        std::future<int&> f = p.get_future();
        assert(test_alloc_base::alloc_count == 1);
        assert(f.valid());
    }
    assert(test_alloc_base::alloc_count == 0);
    {
        std::promise<void> p(std::allocator_arg, test_allocator<void>(42));
        assert(test_alloc_base::alloc_count == 1);
        std::future<void> f = p.get_future();
        assert(test_alloc_base::alloc_count == 1);
        assert(f.valid());
    }
    assert(test_alloc_base::alloc_count == 0);
    // Test with a minimal allocator
    {
        std::promise<int> p(std::allocator_arg, bare_allocator<void>());
        std::future<int> f = p.get_future();
        assert(f.valid());
    }
    {
        std::promise<int&> p(std::allocator_arg, bare_allocator<void>());
        std::future<int&> f = p.get_future();
        assert(f.valid());
    }
    {
        std::promise<void> p(std::allocator_arg, bare_allocator<void>());
        std::future<void> f = p.get_future();
        assert(f.valid());
    }
    // Test with a minimal allocator that returns class-type pointers
    {
        std::promise<int> p(std::allocator_arg, min_allocator<void>());
        std::future<int> f = p.get_future();
        assert(f.valid());
    }
    {
        std::promise<int&> p(std::allocator_arg, min_allocator<void>());
        std::future<int&> f = p.get_future();
        assert(f.valid());
    }
    {
        std::promise<void> p(std::allocator_arg, min_allocator<void>());
        std::future<void> f = p.get_future();
        assert(f.valid());
    }
}
