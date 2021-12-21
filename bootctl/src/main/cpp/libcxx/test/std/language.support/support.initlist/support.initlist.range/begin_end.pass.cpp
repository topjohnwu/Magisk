//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <initializer_list>

// template<class E> const E* begin(initializer_list<E> il);

#include <initializer_list>
#include <cassert>
#include <cstddef>

#include "test_macros.h"

struct A
{
    A(std::initializer_list<int> il)
    {
        const int* b = begin(il);
        const int* e = end(il);
        assert(il.size() == 3);
        assert(static_cast<std::size_t>(e - b) == il.size());
        assert(*b++ == 3);
        assert(*b++ == 2);
        assert(*b++ == 1);
    }
};

#if TEST_STD_VER > 11
struct B
{
    constexpr B(std::initializer_list<int> il)
    {
        const int* b = begin(il);
        const int* e = end(il);
        assert(il.size() == 3);
        assert(static_cast<std::size_t>(e - b) == il.size());
        assert(*b++ == 3);
        assert(*b++ == 2);
        assert(*b++ == 1);
    }
};

#endif  // TEST_STD_VER > 11

int main()
{
    A test1 = {3, 2, 1};
#if TEST_STD_VER > 11
    constexpr B test2 = {3, 2, 1};
    (void)test2;
#endif  // TEST_STD_VER > 11
}
