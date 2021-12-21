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

// void deallocate(pointer p, size_type n);

#include <scoped_allocator>
#include <cassert>

#include "allocators.h"

int main()
{

    {
        typedef std::scoped_allocator_adaptor<A1<int>> A;
        A a;
        a.deallocate((int*)10, 20);
        assert((A1<int>::deallocate_called == std::pair<int*, std::size_t>((int*)10, 20)));
    }
    {
        typedef std::scoped_allocator_adaptor<A1<int>, A2<int>> A;
        A a;
        a.deallocate((int*)10, 20);
        assert((A1<int>::deallocate_called == std::pair<int*, std::size_t>((int*)10, 20)));
    }
    {
        typedef std::scoped_allocator_adaptor<A1<int>, A2<int>, A3<int>> A;
        A a;
        a.deallocate((int*)10, 20);
        assert((A1<int>::deallocate_called == std::pair<int*, std::size_t>((int*)10, 20)));
    }

}
