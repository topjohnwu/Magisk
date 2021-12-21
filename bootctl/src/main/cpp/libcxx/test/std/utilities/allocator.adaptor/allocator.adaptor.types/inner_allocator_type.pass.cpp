//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <memory>

// template <class OuterAlloc, class... InnerAllocs>
//   class scoped_allocator_adaptor

// typedef see below inner_allocator_type;

#include <scoped_allocator>
#include <type_traits>

#include "allocators.h"

int main()
{
    static_assert((std::is_same<
        std::scoped_allocator_adaptor<A1<int>>::inner_allocator_type,
        std::scoped_allocator_adaptor<A1<int>>>::value), "");

    static_assert((std::is_same<
        std::scoped_allocator_adaptor<A1<int>, A2<int>>::inner_allocator_type,
        std::scoped_allocator_adaptor<A2<int>>>::value), "");

    static_assert((std::is_same<
        std::scoped_allocator_adaptor<A1<int>, A2<int>, A3<int>>::inner_allocator_type,
        std::scoped_allocator_adaptor<A2<int>, A3<int>>>::value), "");
}
