//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <experimental/memory_resource>

// template <class Alloc> class resource_adaptor_imp;

// resource_adaptor_imp<Alloc>::resource_adaptor_imp() = default;

#include <experimental/memory_resource>
#include <memory>
#include <type_traits>
#include <cassert>

#include "test_memory_resource.hpp"

namespace ex = std::experimental::pmr;

int main()
{
    {
        typedef CountingAllocator<char> AllocT; // Not default constructible
        typedef ex::resource_adaptor<AllocT> R;
        static_assert(!std::is_default_constructible<R>::value, "");
    }
    {
        typedef std::allocator<char> AllocT; // Is default constructible
        typedef ex::resource_adaptor<AllocT> R;
        static_assert(std::is_default_constructible<R>::value, "");
        R r; ((void)r);
    }
}
