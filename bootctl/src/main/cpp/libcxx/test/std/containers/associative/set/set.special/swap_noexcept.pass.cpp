//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <set>

// void swap(set& c)
//     noexcept(!allocator_type::propagate_on_container_swap::value ||
//              __is_nothrow_swappable<allocator_type>::value);
//
//  In C++17, the standard says that swap shall have:
//     noexcept(allocator_traits<Allocator>::is_always_equal::value &&
//              noexcept(swap(declval<Compare&>(), declval<Compare&>())));

// This tests a conforming extension

#include <set>
#include <utility>
#include <cassert>

#include "test_macros.h"
#include "MoveOnly.h"
#include "test_allocator.h"

template <class T>
struct some_comp
{
    typedef T value_type;

    some_comp() {}
    some_comp(const some_comp&) {}
    bool operator()(const T&, const T&) const { return false; }
};

template <class T>
struct some_comp2
{
    typedef T value_type;

    some_comp2() {}
    some_comp2(const some_comp2&) {}
    bool operator()(const T&, const T&) const { return false; }
};

#if TEST_STD_VER >= 14
template <typename T>
void swap(some_comp2<T>&, some_comp2<T>&) noexcept {}
#endif

template <class T>
struct some_alloc
{
    typedef T value_type;

    some_alloc() {}
    some_alloc(const some_alloc&);
    void deallocate(void*, unsigned) {}

    typedef std::true_type propagate_on_container_swap;
};

template <class T>
struct some_alloc2
{
    typedef T value_type;

    some_alloc2() {}
    some_alloc2(const some_alloc2&);
    void deallocate(void*, unsigned) {}

    typedef std::false_type propagate_on_container_swap;
    typedef std::true_type is_always_equal;
};

template <class T>
struct some_alloc3
{
    typedef T value_type;

    some_alloc3() {}
    some_alloc3(const some_alloc3&);
    void deallocate(void*, unsigned) {}

    typedef std::false_type propagate_on_container_swap;
    typedef std::false_type is_always_equal;
};

int main()
{
    {
        typedef std::set<MoveOnly> C;
        static_assert(noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
#if defined(_LIBCPP_VERSION)
    {
        typedef std::set<MoveOnly, std::less<MoveOnly>, test_allocator<MoveOnly>> C;
        static_assert(noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
    {
        typedef std::set<MoveOnly, std::less<MoveOnly>, other_allocator<MoveOnly>> C;
        static_assert(noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
#endif // _LIBCPP_VERSION
    {
        typedef std::set<MoveOnly, some_comp<MoveOnly>> C;
        static_assert(!noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }

#if TEST_STD_VER >= 14
    { // POCS allocator, throwable swap for comp
    typedef std::set<MoveOnly, some_comp <MoveOnly>, some_alloc <MoveOnly>> C;
    static_assert(!noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
    { // always equal allocator, throwable swap for comp
    typedef std::set<MoveOnly, some_comp <MoveOnly>, some_alloc2<MoveOnly>> C;
    static_assert(!noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
    { // POCS allocator, nothrow swap for comp
    typedef std::set<MoveOnly, some_comp2<MoveOnly>, some_alloc <MoveOnly>> C;
    static_assert( noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
    { // always equal allocator, nothrow swap for comp
    typedef std::set<MoveOnly, some_comp2<MoveOnly>, some_alloc2<MoveOnly>> C;
    static_assert( noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
#if defined(_LIBCPP_VERSION)
    { // NOT always equal allocator, nothrow swap for comp
    typedef std::set<MoveOnly, some_comp2<MoveOnly>, some_alloc3<MoveOnly>> C;
    static_assert( noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
#endif // _LIBCPP_VERSION
#endif

}
