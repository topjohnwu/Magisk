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

// template <class T> class polymorphic_allocator

// template <class U>
// polymorphic_allocator<T>::polymorphic_allocator(polymorphic_allocator<U> const &);


#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

namespace ex = std::experimental::pmr;

int main()
{
    typedef ex::polymorphic_allocator<void> A1;
    typedef ex::polymorphic_allocator<char> A2;
    { // Test that the conversion is implicit and noexcept.
        static_assert(
            std::is_convertible<A1 const &, A2>::value, ""
          );
        static_assert(
            std::is_convertible<A2 const &, A1>::value, ""
          );
        static_assert(
            std::is_nothrow_constructible<A1, A2 const &>::value, ""
          );
        static_assert(
            std::is_nothrow_constructible<A2, A1 const &>::value, ""
          );
    }
    // copy other type
    {
        A1 const a((ex::memory_resource*)42);
        A2 const a2(a);
        assert(a.resource() == a2.resource());
        assert(a2.resource() == (ex::memory_resource*)42);
    }
    {
        A1 a((ex::memory_resource*)42);
        A2 const a2(std::move(a));
        assert(a.resource() == a2.resource());
        assert(a2.resource() == (ex::memory_resource*)42);
    }
}
