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
//     static void deallocate(allocator_type& a, pointer p, size_type n);
//     ...
// };

#include <memory>
#include <cstdint>
#include <cassert>

#include "incomplete_type_helper.h"

int called = 0;

template <class T>
struct A
{
    typedef T value_type;

    void deallocate(value_type* p, std::size_t n)
    {
        assert(p == reinterpret_cast<value_type*>(static_cast<std::uintptr_t>(0xDEADBEEF)));
        assert(n == 10);
        ++called;
    }
};

int main()
{
  {
    A<int> a;
    std::allocator_traits<A<int> >::deallocate(a, reinterpret_cast<int*>(static_cast<std::uintptr_t>(0xDEADBEEF)), 10);
    assert(called == 1);
  }
  called = 0;
  {
    typedef IncompleteHolder* VT;
    typedef A<VT> Alloc;
    Alloc a;
    std::allocator_traits<Alloc >::deallocate(a, reinterpret_cast<VT*>(static_cast<std::uintptr_t>(0xDEADBEEF)), 10);
    assert(called == 1);
  }
}
