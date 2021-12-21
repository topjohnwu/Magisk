//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <string>

// void swap(basic_string& c)
//     noexcept(!allocator_type::propagate_on_container_swap::value ||
//              __is_nothrow_swappable<allocator_type>::value);
//
//  In C++17, the standard says that swap shall have:
//     noexcept(allocator_traits<Allocator>::propagate_on_container_swap::value ||
//              allocator_traits<Allocator>::is_always_equal::value);

// This tests a conforming extension

#include <string>
#include <utility>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"

template <class T>
struct some_alloc
{
    typedef T value_type;

    some_alloc() {}
    some_alloc(const some_alloc&);
    T *allocate(size_t);
    void deallocate(void*, unsigned) {}
    typedef std::true_type propagate_on_container_swap;
};

template <class T>
struct some_alloc2
{
    typedef T value_type;

    some_alloc2() {}
    some_alloc2(const some_alloc2&);
    T *allocate(size_t);
    void deallocate(void*, unsigned) {}

    typedef std::false_type propagate_on_container_swap;
    typedef std::true_type is_always_equal;
};

int main()
{
    {
        typedef std::string C;
        static_assert(noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
#if defined(_LIBCPP_VERSION)
    {
        typedef std::basic_string<char, std::char_traits<char>, test_allocator<char>> C;
        static_assert(noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
#endif // _LIBCPP_VERSION
    {
        typedef std::basic_string<char, std::char_traits<char>, some_alloc<char>> C;
#if TEST_STD_VER >= 14
    //  In C++14, if POCS is set, swapping the allocator is required not to throw
        static_assert( noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
#else
        static_assert(!noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
#endif
    }
#if TEST_STD_VER >= 14
    {
        typedef std::basic_string<char, std::char_traits<char>, some_alloc2<char>> C;
    //  if the allocators are always equal, then the swap can be noexcept
        static_assert( noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
#endif
}
