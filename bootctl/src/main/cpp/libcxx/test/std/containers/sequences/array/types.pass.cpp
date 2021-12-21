//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// template <class T, size_t N >
// struct array
// {
//     // types:
//     typedef T& reference;
//     typedef const T& const_reference;
//     typedef implementation defined iterator;
//     typedef implementation defined const_iterator;
//     typedef T value_type;
//     typedef T* pointer;
//     typedef size_t size_type;
//     typedef ptrdiff_t difference_type;
//     typedef T value_type;
//     typedef std::reverse_iterator<iterator> reverse_iterator;
//     typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

#include <array>
#include <iterator>
#include <type_traits>

#include "test_macros.h"

template <class C>
void test_iterators() {
    typedef std::iterator_traits<typename C::iterator> ItT;
    typedef std::iterator_traits<typename C::const_iterator> CItT;
    static_assert((std::is_same<typename ItT::iterator_category, std::random_access_iterator_tag>::value), "");
    static_assert((std::is_same<typename ItT::value_type, typename C::value_type>::value), "");
    static_assert((std::is_same<typename ItT::reference, typename C::reference>::value), "");
    static_assert((std::is_same<typename ItT::pointer, typename C::pointer>::value), "");
    static_assert((std::is_same<typename ItT::difference_type, typename C::difference_type>::value), "");

    static_assert((std::is_same<typename CItT::iterator_category, std::random_access_iterator_tag>::value), "");
    static_assert((std::is_same<typename CItT::value_type, typename C::value_type>::value), "");
    static_assert((std::is_same<typename CItT::reference, typename C::const_reference>::value), "");
    static_assert((std::is_same<typename CItT::pointer, typename C::const_pointer>::value), "");
    static_assert((std::is_same<typename CItT::difference_type, typename C::difference_type>::value), "");
}

int main()
{
    {
        typedef double T;
        typedef std::array<T, 10> C;
        static_assert((std::is_same<C::reference, T&>::value), "");
        static_assert((std::is_same<C::const_reference, const T&>::value), "");
        LIBCPP_STATIC_ASSERT((std::is_same<C::iterator, T*>::value), "");
        LIBCPP_STATIC_ASSERT((std::is_same<C::const_iterator, const T*>::value), "");
        test_iterators<C>();
        static_assert((std::is_same<C::pointer, T*>::value), "");
        static_assert((std::is_same<C::const_pointer, const T*>::value), "");
        static_assert((std::is_same<C::size_type, std::size_t>::value), "");
        static_assert((std::is_same<C::difference_type, std::ptrdiff_t>::value), "");
        static_assert((std::is_same<C::reverse_iterator, std::reverse_iterator<C::iterator> >::value), "");
        static_assert((std::is_same<C::const_reverse_iterator, std::reverse_iterator<C::const_iterator> >::value), "");

        static_assert((std::is_signed<typename C::difference_type>::value), "");
        static_assert((std::is_unsigned<typename C::size_type>::value), "");
        static_assert((std::is_same<typename C::difference_type,
            typename std::iterator_traits<typename C::iterator>::difference_type>::value), "");
        static_assert((std::is_same<typename C::difference_type,
            typename std::iterator_traits<typename C::const_iterator>::difference_type>::value), "");
    }
    {
        typedef int* T;
        typedef std::array<T, 0> C;
        static_assert((std::is_same<C::reference, T&>::value), "");
        static_assert((std::is_same<C::const_reference, const T&>::value), "");
        LIBCPP_STATIC_ASSERT((std::is_same<C::iterator, T*>::value), "");
        LIBCPP_STATIC_ASSERT((std::is_same<C::const_iterator, const T*>::value), "");
        test_iterators<C>();
        static_assert((std::is_same<C::pointer, T*>::value), "");
        static_assert((std::is_same<C::const_pointer, const T*>::value), "");
        static_assert((std::is_same<C::size_type, std::size_t>::value), "");
        static_assert((std::is_same<C::difference_type, std::ptrdiff_t>::value), "");
        static_assert((std::is_same<C::reverse_iterator, std::reverse_iterator<C::iterator> >::value), "");
        static_assert((std::is_same<C::const_reverse_iterator, std::reverse_iterator<C::const_iterator> >::value), "");

        static_assert((std::is_signed<typename C::difference_type>::value), "");
        static_assert((std::is_unsigned<typename C::size_type>::value), "");
        static_assert((std::is_same<typename C::difference_type,
            typename std::iterator_traits<typename C::iterator>::difference_type>::value), "");
        static_assert((std::is_same<typename C::difference_type,
            typename std::iterator_traits<typename C::const_iterator>::difference_type>::value), "");
    }
}
