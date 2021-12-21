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

// typedef see below propagate_on_container_move_assignment;

#include <scoped_allocator>
#include <type_traits>

#include "allocators.h"

int main()
{
    static_assert((std::is_same<
        std::scoped_allocator_adaptor<A1<int>>::propagate_on_container_move_assignment,
        std::false_type>::value), "");

    static_assert((std::is_same<
        std::scoped_allocator_adaptor<A1<int>, A2<int>>::propagate_on_container_move_assignment,
        std::true_type>::value), "");

    static_assert((std::is_same<
        std::scoped_allocator_adaptor<A1<int>, A2<int>, A3<int>>::propagate_on_container_move_assignment,
        std::true_type>::value), "");

}
