//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class Alloc>
// struct allocator_traits
// {
//     static pointer allocate(allocator_type& a, size_type n);
//     ...
// };

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17
// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5, clang-3.6, clang-3.7, clang-3.8

#include <memory>
#include <cstdint>
#include <cassert>

#include "test_macros.h"

template <class T>
struct A
{
    typedef T value_type;

    value_type* allocate(std::size_t n)
    {
        assert(n == 12);
        return reinterpret_cast<value_type*>(static_cast<std::uintptr_t>(0xEEADBEEF));
    }
    value_type* allocate(std::size_t n, const void* p)
    {
        assert(n == 11);
        assert(p == 0);
        return reinterpret_cast<value_type*>(static_cast<std::uintptr_t>(0xFEADBEEF));
    }
};

int main()
{
    A<int> a;
    std::allocator_traits<A<int> >::allocate(a, 10);          // expected-error {{ignoring return value of function declared with 'nodiscard' attribute}}
    std::allocator_traits<A<int> >::allocate(a, 10, nullptr); // expected-error {{ignoring return value of function declared with 'nodiscard' attribute}}
}
