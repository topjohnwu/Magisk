//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// vector(vector&&)
//        noexcept(is_nothrow_move_constructible<allocator_type>::value);

// This tests a conforming extension

// UNSUPPORTED: c++98, c++03

#include <vector>
#include <cassert>

#include "MoveOnly.h"
#include "test_allocator.h"

template <class T>
struct some_alloc
{
    typedef T value_type;
    some_alloc(const some_alloc&);
};

int main()
{
    {
        typedef std::vector<MoveOnly> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::vector<MoveOnly, test_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::vector<MoveOnly, other_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::vector<MoveOnly, some_alloc<MoveOnly>> C;
    //  In C++17, move constructors for allocators are not allowed to throw
#if TEST_STD_VER > 14
        static_assert( std::is_nothrow_move_constructible<C>::value, "");
#else
        static_assert(!std::is_nothrow_move_constructible<C>::value, "");
#endif
    }
}
