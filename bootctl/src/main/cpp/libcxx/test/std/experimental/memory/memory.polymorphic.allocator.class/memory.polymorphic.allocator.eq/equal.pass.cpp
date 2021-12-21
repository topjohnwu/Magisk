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

// template <class T> class polymorphic_allocator;

// template <class T, class U>
// bool operator==(
//      polymorphic_allocator<T> const &
//    , polymorphic_allocator<U> const &) noexcept

#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

#include "test_memory_resource.hpp"

namespace ex = std::experimental::pmr;

int main()
{
    typedef ex::polymorphic_allocator<void> A1;
    typedef ex::polymorphic_allocator<int> A2;
    // check return types
    {
        A1 const a1;
        A2 const a2;
        static_assert(std::is_same<decltype(a1 == a2), bool>::value, "");
        static_assert(noexcept(a1 == a2), "");
    }
    // equal same type (different resource)
    {
        TestResource d1(1);
        TestResource d2(1);
        A1 const a1(&d1);
        A1 const a2(&d2);

        assert(a1 == a2);
        assert(d1.checkIsEqualCalledEq(1));
        assert(d2.checkIsEqualCalledEq(0));

        d1.reset();

        assert(a2 == a1);
        assert(d1.checkIsEqualCalledEq(0));
        assert(d2.checkIsEqualCalledEq(1));
    }
    // equal same type (same resource)
    {
        TestResource d1;
        A1 const a1(&d1);
        A1 const a2(&d1);

        assert(a1 == a2);
        assert(d1.checkIsEqualCalledEq(0));

        assert(a2 == a1);
        assert(d1.checkIsEqualCalledEq(0));
    }
    // equal different type (different resource)
    {
        TestResource d1(42);
        TestResource d2(42);
        A1 const a1(&d1);
        A2 const a2(&d2);

        assert(a1 == a2);
        assert(d1.checkIsEqualCalledEq(1));
        assert(d2.checkIsEqualCalledEq(0));

        assert(a2 == a1);
        assert(d1.checkIsEqualCalledEq(1));
        assert(d2.checkIsEqualCalledEq(1));

    }
    // equal different type (same resource)
    {
        TestResource d1(42);
        A1 const a1(&d1);
        A2 const a2(&d1);

        assert(a1 == a2);
        assert(d1.checkIsEqualCalledEq(0));

        assert(a2 == a1);
        assert(d1.checkIsEqualCalledEq(0));

    }
    // not equal same type
    {
        TestResource d1(1);
        TestResource d2(2);
        A1 const a1(&d1);
        A1 const a2(&d2);

        assert(!(a1 == a2));
        assert(d1.checkIsEqualCalledEq(1));
        assert(d2.checkIsEqualCalledEq(0));

        d1.reset();

        assert(!(a2 == a1));
        assert(d1.checkIsEqualCalledEq(0));
        assert(d2.checkIsEqualCalledEq(1));

    }
    // not equal different types
    {
        TestResource  d1;
        TestResource1 d2;
        A1 const a1(&d1);
        A2 const a2(&d2);

        assert(!(a1 == a2));
        assert(d1.checkIsEqualCalledEq(1));
        assert(d2.checkIsEqualCalledEq(0));

        d1.reset();

        assert(!(a2 == a1));
        assert(d1.checkIsEqualCalledEq(0));
        assert(d2.checkIsEqualCalledEq(1));
    }
}
