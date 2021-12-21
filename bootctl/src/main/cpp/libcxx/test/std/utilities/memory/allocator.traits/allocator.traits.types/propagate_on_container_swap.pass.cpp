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
//     typedef Alloc::propagate_on_container_swap
//           | false_type                   propagate_on_container_swap;
//     ...
// };

#include <memory>
#include <type_traits>

#include "test_macros.h"

template <class T>
struct A
{
    typedef T value_type;
    typedef std::true_type propagate_on_container_swap;
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
private:
    typedef std::true_type propagate_on_container_swap;
};

int main()
{
    static_assert((std::is_same<std::allocator_traits<A<char> >::propagate_on_container_swap, std::true_type>::value), "");
    static_assert((std::is_same<std::allocator_traits<B<char> >::propagate_on_container_swap, std::false_type>::value), "");
 #if TEST_STD_VER >= 11
    static_assert((std::is_same<std::allocator_traits<C<char> >::propagate_on_container_swap, std::false_type>::value), "");
#endif
}
