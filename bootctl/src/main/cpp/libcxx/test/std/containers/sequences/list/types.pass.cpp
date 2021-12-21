//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// template <class T, class Alloc = allocator<T> >
// class list
// {
// public:
//
//     // types:
//     typedef T value_type;
//     typedef Alloc allocator_type;
//     typedef typename allocator_type::reference reference;
//     typedef typename allocator_type::const_reference const_reference;
//     typedef typename allocator_type::pointer pointer;
//     typedef typename allocator_type::const_pointer const_pointer;

#include <list>
#include <type_traits>

#include "min_allocator.h"

struct A { std::list<A> v; }; // incomplete type support

int main()
{
    {
    typedef std::list<int> C;
    static_assert((std::is_same<C::value_type, int>::value), "");
    static_assert((std::is_same<C::allocator_type, std::allocator<int> >::value), "");
    static_assert((std::is_same<C::reference, std::allocator<int>::reference>::value), "");
    static_assert((std::is_same<C::const_reference, std::allocator<int>::const_reference>::value), "");
    static_assert((std::is_same<C::pointer, std::allocator<int>::pointer>::value), "");
    static_assert((std::is_same<C::const_pointer, std::allocator<int>::const_pointer>::value), "");

    static_assert((std::is_signed<typename C::difference_type>::value), "");
    static_assert((std::is_unsigned<typename C::size_type>::value), "");
    static_assert((std::is_same<typename C::difference_type,
        typename std::iterator_traits<typename C::iterator>::difference_type>::value), "");
    static_assert((std::is_same<typename C::difference_type,
        typename std::iterator_traits<typename C::const_iterator>::difference_type>::value), "");
    }

#if TEST_STD_VER >= 11
    {
    typedef std::list<int, min_allocator<int>> C;
    static_assert((std::is_same<C::value_type, int>::value), "");
    static_assert((std::is_same<C::allocator_type, min_allocator<int> >::value), "");
    static_assert((std::is_same<C::reference, int&>::value), "");
    static_assert((std::is_same<C::const_reference, const int&>::value), "");
    static_assert((std::is_same<C::pointer, min_pointer<int>>::value), "");
    static_assert((std::is_same<C::const_pointer, min_pointer<const int>>::value), "");

    static_assert((std::is_signed<typename C::difference_type>::value), "");
    static_assert((std::is_unsigned<typename C::size_type>::value), "");
    static_assert((std::is_same<typename C::difference_type,
        typename std::iterator_traits<typename C::iterator>::difference_type>::value), "");
    static_assert((std::is_same<typename C::difference_type,
        typename std::iterator_traits<typename C::const_iterator>::difference_type>::value), "");
    }
#endif
}
