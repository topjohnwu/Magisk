//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17
// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5, clang-3.6, clang-3.7, clang-3.8

// <memory>

// template <class OuterAlloc, class... InnerAllocs>
//   class scoped_allocator_adaptor

// pointer allocate(size_type n, const_void_pointer hint);

#include <scoped_allocator>
#include <cassert>

#include "allocators.h"

int main()
{
    std::scoped_allocator_adaptor<A1<int>> a;
    a.allocate(10, (const void*)0); // expected-error {{ignoring return value of function declared with 'nodiscard' attribute}}
}
