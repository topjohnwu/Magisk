//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// An array is a contiguous container

#include <array>
#include <cassert>

template <class C>
void test_contiguous ( const C &c )
{
    for ( size_t i = 0; i < c.size(); ++i )
        assert ( *(c.begin() + i) == *(std::addressof(*c.begin()) + i));
}

int main()
{
    {
        typedef double T;
        typedef std::array<T, 3> C;
        test_contiguous (C());
    }
}
