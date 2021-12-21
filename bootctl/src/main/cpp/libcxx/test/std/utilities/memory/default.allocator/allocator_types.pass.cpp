//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// check nested types:

// template <class T>
// class allocator
// {
// public:
//     typedef size_t                                size_type;
//     typedef ptrdiff_t                             difference_type;
//     typedef T*                                    pointer;
//     typedef const T*                              const_pointer;
//     typedef typename add_lvalue_reference<T>::type       reference;
//     typedef typename add_lvalue_reference<const T>::type const_reference;
//     typedef T                                     value_type;
//     typedef true_type                             is_always_equal;
//
//     template <class U> struct rebind {typedef allocator<U> other;};
// ...
// };

#include <memory>
#include <type_traits>
#include <cstddef>

#include "test_macros.h"

int main()
{
    static_assert((std::is_same<std::allocator<char>::size_type, std::size_t>::value), "");
    static_assert((std::is_same<std::allocator<char>::difference_type, std::ptrdiff_t>::value), "");
    static_assert((std::is_same<std::allocator<char>::pointer, char*>::value), "");
    static_assert((std::is_same<std::allocator<char>::const_pointer, const char*>::value), "");
    static_assert((std::is_same<std::allocator<char>::value_type, char>::value), "");
    static_assert((std::is_same<std::allocator<char>::reference, char&>::value), "");
    static_assert((std::is_same<std::allocator<char>::const_reference, const char&>::value), "");
    static_assert((std::is_same<std::allocator<char>::rebind<int>::other,
                                std::allocator<int> >::value), "");

    static_assert((std::is_same<std::allocator<      char>::is_always_equal, std::true_type>::value), "");
    LIBCPP_STATIC_ASSERT((std::is_same<std::allocator<const char>::is_always_equal, std::true_type>::value), "");

    std::allocator<char> a;
    std::allocator<char> a2 = a;
    a2 = a;
    std::allocator<int> a3 = a2;
    ((void)a3);
}
