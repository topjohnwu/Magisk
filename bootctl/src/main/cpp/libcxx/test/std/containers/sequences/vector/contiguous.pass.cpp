//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// An vector is a contiguous container

#include <vector>
#include <cassert>

#include "test_allocator.h"
#include "min_allocator.h"

template <class C>
void test_contiguous ( const C &c )
{
    for ( size_t i = 0; i < c.size(); ++i )
        assert ( *(c.begin() + static_cast<typename C::difference_type>(i)) == *(std::addressof(*c.begin()) + i));
}

int main()
{
    {
    typedef int T;
    typedef std::vector<T> C;
    test_contiguous(C());
    test_contiguous(C(3, 5));
    }

    {
    typedef double T;
    typedef test_allocator<T> A;
    typedef std::vector<T, A> C;
    test_contiguous(C(A(3)));
    test_contiguous(C(7, 9.0, A(5)));
    }
#if TEST_STD_VER >= 11
    {
    typedef double T;
    typedef min_allocator<T> A;
    typedef std::vector<T, A> C;
    test_contiguous(C(A{}));
    test_contiguous(C(9, 11.0, A{}));
    }
#endif
}
