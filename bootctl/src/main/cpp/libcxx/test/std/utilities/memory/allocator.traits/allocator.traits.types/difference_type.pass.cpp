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
//     typedef Alloc::difference_type
//           | pointer_traits<pointer>::difference_type         difference_type;
//     ...
// };

#include <memory>
#include <type_traits>

#include "test_macros.h"

template <class T>
struct A
{
    typedef T value_type;
    typedef short difference_type;
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
struct D
{
    typedef T value_type;
private:
    typedef void difference_type;
};

namespace std
{

template <>
struct pointer_traits<C<char>::pointer>
{
    typedef C<char>::pointer pointer;
    typedef char             element_type;
    typedef signed char      difference_type;
};

}

int main()
{
    static_assert((std::is_same<std::allocator_traits<A<char> >::difference_type, short>::value), "");
    static_assert((std::is_same<std::allocator_traits<B<char> >::difference_type, std::ptrdiff_t>::value), "");
    static_assert((std::is_same<std::allocator_traits<C<char> >::difference_type, signed char>::value), "");
#if TEST_STD_VER >= 11
    static_assert((std::is_same<std::allocator_traits<D<char> >::difference_type, std::ptrdiff_t>::value), "");
#endif
}
