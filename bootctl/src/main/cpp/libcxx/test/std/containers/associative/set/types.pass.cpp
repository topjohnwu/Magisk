//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// template <class Key, class Compare = less<Key>,
//           class Allocator = allocator<Key>>
// class set
// {
// public:
//     // types:
//     typedef Key                                      key_type;
//     typedef key_type                                 value_type;
//     typedef Compare                                  key_compare;
//     typedef key_compare                              value_compare;
//     typedef Allocator                                allocator_type;
//     typedef typename allocator_type::reference       reference;
//     typedef typename allocator_type::const_reference const_reference;
//     typedef typename allocator_type::pointer         pointer;
//     typedef typename allocator_type::const_pointer   const_pointer;
//     typedef typename allocator_type::size_type       size_type;
//     typedef typename allocator_type::difference_type difference_type;
//     ...
// };

#include <set>
#include <type_traits>

#include "min_allocator.h"

int main()
{
    {
    typedef std::set<int> C;
    static_assert((std::is_same<C::key_type, int>::value), "");
    static_assert((std::is_same<C::value_type, int>::value), "");
    static_assert((std::is_same<C::key_compare, std::less<int> >::value), "");
    static_assert((std::is_same<C::value_compare, std::less<int> >::value), "");
    static_assert((std::is_same<C::allocator_type, std::allocator<int> >::value), "");
    static_assert((std::is_same<C::reference, int&>::value), "");
    static_assert((std::is_same<C::const_reference, const int&>::value), "");
    static_assert((std::is_same<C::pointer, int*>::value), "");
    static_assert((std::is_same<C::const_pointer, const int*>::value), "");
    static_assert((std::is_same<C::size_type, std::size_t>::value), "");
    static_assert((std::is_same<C::difference_type, std::ptrdiff_t>::value), "");
    }
#if TEST_STD_VER >= 11
    {
    typedef std::set<int, std::less<int>, min_allocator<int>> C;
    static_assert((std::is_same<C::key_type, int>::value), "");
    static_assert((std::is_same<C::value_type, int>::value), "");
    static_assert((std::is_same<C::key_compare, std::less<int> >::value), "");
    static_assert((std::is_same<C::value_compare, std::less<int> >::value), "");
    static_assert((std::is_same<C::allocator_type, min_allocator<int> >::value), "");
    static_assert((std::is_same<C::reference, int&>::value), "");
    static_assert((std::is_same<C::const_reference, const int&>::value), "");
    static_assert((std::is_same<C::pointer, min_pointer<int>>::value), "");
    static_assert((std::is_same<C::const_pointer, min_pointer<const int>>::value), "");
//  min_allocator doesn't have a size_type, so one gets synthesized
    static_assert((std::is_same<C::size_type, std::make_unsigned<C::difference_type>::type>::value), "");
    static_assert((std::is_same<C::difference_type, std::ptrdiff_t>::value), "");
    }
#endif
}
