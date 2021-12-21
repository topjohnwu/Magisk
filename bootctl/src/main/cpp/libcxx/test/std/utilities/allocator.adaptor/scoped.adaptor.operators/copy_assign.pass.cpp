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

// scoped_allocator_adaptor& operator=(const scoped_allocator_adaptor& other);


#include <scoped_allocator>
#include <cassert>

#include "allocators.h"

int main()
{
    {
        typedef std::scoped_allocator_adaptor<A1<int>> A;
        A a1(A1<int>(3));
        A aN;
        A1<int>::copy_called = false;
        A1<int>::move_called = false;
        aN = a1;
        assert(A1<int>::copy_called == true);
        assert(A1<int>::move_called == false);
        assert(aN == a1);
    }
    {
        typedef std::scoped_allocator_adaptor<A1<int>, A2<int>> A;
        A a1(A1<int>(4), A2<int>(5));
        A aN;
        A1<int>::copy_called = false;
        A1<int>::move_called = false;
        A2<int>::copy_called = false;
        A2<int>::move_called = false;
        aN = a1;
        assert(A1<int>::copy_called == true);
        assert(A1<int>::move_called == false);
        assert(A2<int>::copy_called == true);
        assert(A2<int>::move_called == false);
        assert(aN == a1);
    }
    {
        typedef std::scoped_allocator_adaptor<A1<int>, A2<int>, A3<int>> A;
        A a1(A1<int>(4), A2<int>(5), A3<int>(6));
        A aN;
        A1<int>::copy_called = false;
        A1<int>::move_called = false;
        A2<int>::copy_called = false;
        A2<int>::move_called = false;
        A3<int>::copy_called = false;
        A3<int>::move_called = false;
        aN = a1;
        assert(A1<int>::copy_called == true);
        assert(A1<int>::move_called == false);
        assert(A2<int>::copy_called == true);
        assert(A2<int>::move_called == false);
        assert(A3<int>::copy_called == true);
        assert(A3<int>::move_called == false);
        assert(aN == a1);
    }
}
