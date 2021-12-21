//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// vector& operator=(vector&& c)
//     noexcept(
//          allocator_type::propagate_on_container_move_assignment::value &&
//          is_nothrow_move_assignable<allocator_type>::value);

// This tests a conforming extension

// UNSUPPORTED: c++98, c++03

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

template <class T>
struct some_alloc2
{
    typedef T value_type;

    some_alloc2() {}
    some_alloc2(const some_alloc2&);
    void deallocate(void*, unsigned) {}

    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::true_type is_always_equal;
};

template <class T>
struct some_alloc3
{
    typedef T value_type;

    some_alloc3() {}
    some_alloc3(const some_alloc3&);
    void deallocate(void*, unsigned) {}

    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::false_type is_always_equal;
};

int main()
{
#if defined(_LIBCPP_VERSION)
    {
        typedef std::vector<bool> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
#endif // _LIBCPP_VERSION
    {
        typedef std::vector<bool, test_allocator<bool>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
#if defined(_LIBCPP_VERSION)
    {
        typedef std::vector<bool, other_allocator<bool>> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
#endif // _LIBCPP_VERSION
    {
#if TEST_STD_VER > 14
#if defined(_LIBCPP_VERSION)
        typedef std::vector<bool, some_alloc<bool>> C;
        static_assert( std::is_nothrow_move_assignable<C>::value, "");
#endif // _LIBCPP_VERSION
#else
        typedef std::vector<bool, some_alloc<bool>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
#endif
    }
#if TEST_STD_VER > 14
#if defined(_LIBCPP_VERSION)
    {  // POCMA false, is_always_equal true
        typedef std::vector<bool, some_alloc2<bool>> C;
        static_assert( std::is_nothrow_move_assignable<C>::value, "");
    }
#endif // _LIBCPP_VERSION
    {  // POCMA false, is_always_equal false
        typedef std::vector<bool, some_alloc3<bool>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
#endif
}
