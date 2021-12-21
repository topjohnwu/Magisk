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

// inner_allocator_type& inner_allocator();
// const inner_allocator_type& inner_allocator() const;

#include <scoped_allocator>
#include <cassert>

#include "allocators.h"

int main()
{
    {
        typedef std::scoped_allocator_adaptor<A1<int>> A;
        A a(A1<int>(5));
        assert(a.inner_allocator() == a);
    }
    {
        typedef std::scoped_allocator_adaptor<A1<int>, A2<int>> A;
        A a(A1<int>(5), A2<int>(6));
        assert(a.inner_allocator() == std::scoped_allocator_adaptor<A2<int>>(A2<int>(6)));
    }
    {
        typedef std::scoped_allocator_adaptor<A1<int>, A2<int>, A3<int>> A;
        A a(A1<int>(5), A2<int>(6), A3<int>(8));
        assert((a.inner_allocator() ==
            std::scoped_allocator_adaptor<A2<int>, A3<int>>(A2<int>(6), A3<int>(8))));
    }

}
