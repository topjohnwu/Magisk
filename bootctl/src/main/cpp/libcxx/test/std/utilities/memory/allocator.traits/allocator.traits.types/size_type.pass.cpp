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
//     typedef Alloc::size_type | size_t    size_type;
//     ...
// };

#include <memory>
#include <type_traits>

#include "test_macros.h"

template <class T>
struct A
{
    typedef T value_type;
    typedef unsigned short size_type;
};

template <class T>
struct B
{
    typedef T value_type;
};

template <class T>
struct C
{
    typedef T value_type;
    struct pointer {};
    struct const_pointer {};
    struct void_pointer {};
    struct const_void_pointer {};
};

template <class T>
struct D {
    typedef T value_type;
    typedef short difference_type;
private:
    typedef void size_type;
};

namespace std
{

template <>
struct pointer_traits<C<char>::pointer>
{
    typedef signed char difference_type;
};

}

int main()
{
    static_assert((std::is_same<std::allocator_traits<A<char> >::size_type, unsigned short>::value), "");
    static_assert((std::is_same<std::allocator_traits<B<char> >::size_type,
                   std::make_unsigned<std::ptrdiff_t>::type>::value), "");
    static_assert((std::is_same<std::allocator_traits<C<char> >::size_type,
                   unsigned char>::value), "");
#if TEST_STD_VER >= 11
    static_assert((std::is_same<std::allocator_traits<D<char> >::size_type, unsigned short>::value), "");
#endif
}
