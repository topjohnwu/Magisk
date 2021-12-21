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

// template <class OuterA1, class OuterA2, class... InnerAllocs>
//     bool
//     operator==(const scoped_allocator_adaptor<OuterA1, InnerAllocs...>& a,
//                const scoped_allocator_adaptor<OuterA2, InnerAllocs...>& b);
//
// template <class OuterA1, class OuterA2, class... InnerAllocs>
//     bool
//     operator!=(const scoped_allocator_adaptor<OuterA1, InnerAllocs...>& a,
//                const scoped_allocator_adaptor<OuterA2, InnerAllocs...>& b);

#include <scoped_allocator>
#include <cassert>

#include "allocators.h"

int main()
{
    {
        typedef std::scoped_allocator_adaptor<A1<int>> A;
        A a1(A1<int>(3));
        A a2 = a1;
        assert(a2 == a1);
        assert(!(a2 != a1));
    }
    {
        typedef std::scoped_allocator_adaptor<A1<int>, A2<int>> A;
        A a1(A1<int>(4), A2<int>(5));
        A a2 = a1;
        assert(a2 == a1);
        assert(!(a2 != a1));
    }
    {
        typedef std::scoped_allocator_adaptor<A1<int>, A2<int>, A3<int>> A;
        A a1(A1<int>(4), A2<int>(5), A3<int>(6));
        A a2 = a1;
        assert(a2 == a1);
        assert(!(a2 != a1));
    }
    {
        typedef std::scoped_allocator_adaptor<A1<int>, A2<int>, A3<int>> A;
        A a1(A1<int>(4), A2<int>(5), A3<int>(6));
        A a2(A1<int>(4), A2<int>(5), A3<int>(5));
        assert(a2 != a1);
        assert(!(a2 == a1));
    }
}
