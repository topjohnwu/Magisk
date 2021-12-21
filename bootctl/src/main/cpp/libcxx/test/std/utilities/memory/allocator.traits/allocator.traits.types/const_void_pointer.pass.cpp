//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class Alloc>
// struct allocator_traits
// {
//     typedef Alloc::const_void_pointer
//           | pointer_traits<pointer>::rebind<const void>
//                                          const_void_pointer;
//     ...
// };

#include <memory>
#include <type_traits>

#include "test_macros.h"

template <class T>
struct Ptr {};

template <class T>
struct A
{
    typedef T value_type;
    typedef Ptr<T> pointer;
};

template <class T>
struct B
{
    typedef T value_type;
};

template <class T>
struct CPtr {};

template <class T>
struct C
{
    typedef T value_type;
    typedef CPtr<const void> const_void_pointer;
};


template <class T>
struct D
{
    typedef T value_type;
private:
    typedef int const_void_pointer;
};

int main()
{
    static_assert((std::is_same<std::allocator_traits<A<char> >::const_void_pointer, Ptr<const void> >::value), "");
    static_assert((std::is_same<std::allocator_traits<B<char> >::const_void_pointer, const void*>::value), "");
    static_assert((std::is_same<std::allocator_traits<C<char> >::const_void_pointer, CPtr<const void> >::value), "");
#if TEST_STD_VER >= 11
    static_assert((std::is_same<std::allocator_traits<D<char> >::const_void_pointer, const void*>::value), "");
#endif
}
