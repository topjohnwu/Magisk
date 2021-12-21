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

#include <experimental/memory_resource>
#include <type_traits>
#include <memory>
#include <cassert>

namespace ex = std::experimental::pmr;

int main()
{
    typedef ex::resource_adaptor<std::allocator<void>> R;
    typedef ex::resource_adaptor<std::allocator<long>> R2;
    static_assert(std::is_same<R, R2>::value, "");
    {
        static_assert(std::is_base_of<ex::memory_resource, R>::value, "");
        static_assert(std::is_same<R::allocator_type, std::allocator<char>>::value, "");
    }
    {
        static_assert(std::is_default_constructible<R>::value, "");
        static_assert(std::is_copy_constructible<R>::value, "");
        static_assert(std::is_move_constructible<R>::value, "");
        static_assert(std::is_copy_assignable<R>::value, "");
        static_assert(std::is_move_assignable<R>::value, "");
   }
}
