//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03

// <vector>

// vector<bool>()
//        noexcept(is_nothrow_default_constructible<allocator_type>::value);

// This tests a conforming extension
// For vector<>, this was added to the standard by N4258,
//   but vector<bool> was not changed.


#include <vector>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"

template <class T>
struct some_alloc
{
    typedef T value_type;
    some_alloc(const some_alloc&);
};

int main()
{
#if defined(_LIBCPP_VERSION)
    {
        typedef std::vector<bool> C;
        static_assert(std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef std::vector<bool, test_allocator<bool>> C;
        static_assert(std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef std::vector<bool, other_allocator<bool>> C;
        static_assert(!std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef std::vector<bool, some_alloc<bool>> C;
        static_assert(!std::is_nothrow_default_constructible<C>::value, "");
    }
#endif // _LIBCPP_VERSION
}
