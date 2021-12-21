//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: c++experimental
// UNSUPPORTED: c++98, c++03

// <experimental/memory_resource>

// template <class T> class polymorphic_allocator

// polymorphic_allocator
// polymorphic_allocator<T>::select_on_container_copy_construction() const

#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

namespace ex = std::experimental::pmr;

int main()
{
    typedef ex::polymorphic_allocator<void> A;
    {
        A const a;
        static_assert(
            std::is_same<decltype(a.select_on_container_copy_construction()), A>::value,
            "");
    }
    {
        ex::memory_resource * mptr = (ex::memory_resource*)42;
        A const a(mptr);
        assert(a.resource() == mptr);
        A const other = a.select_on_container_copy_construction();
        assert(other.resource() == ex::get_default_resource());
        assert(a.resource() == mptr);
    }
    {
        ex::memory_resource * mptr = (ex::memory_resource*)42;
        ex::set_default_resource(mptr);
        A const a(nullptr);
        assert(a.resource() == nullptr);
        A const other = a.select_on_container_copy_construction();
        assert(other.resource() == ex::get_default_resource());
        assert(other.resource() == mptr);
        assert(a.resource() == nullptr);
    }
}
