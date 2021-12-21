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

// memory_resource *
// polymorphic_allocator<T>::resource() const

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
            std::is_same<decltype(a.resource()), ex::memory_resource*>::value
          , ""
          );
    }
    {
        ex::memory_resource * mptr = (ex::memory_resource*)42;
        A const a(mptr);
        assert(a.resource() == mptr);
    }
    {
        A const a(nullptr);
        assert(a.resource() == nullptr);
        assert(a.resource() == nullptr);
    }
    {
        A const a;
        assert(a.resource() == ex::get_default_resource());
    }
    {
        ex::memory_resource * mptr = (ex::memory_resource*)42;
        ex::set_default_resource(mptr);
        A const a;
        assert(a.resource() == mptr);
        assert(a.resource() == ex::get_default_resource());
    }
}
