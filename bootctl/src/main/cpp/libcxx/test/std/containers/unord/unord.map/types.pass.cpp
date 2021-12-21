//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map
// {
// public:
//     // types
//     typedef Key                                                        key_type;
//     typedef T                                                          mapped_type;
//     typedef Hash                                                       hasher;
//     typedef Pred                                                       key_equal;
//     typedef Alloc                                                      allocator_type;
//     typedef pair<const key_type, mapped_type>                          value_type;
//     typedef value_type&                                                reference;
//     typedef const value_type&                                          const_reference;
//     typedef typename allocator_traits<allocator_type>::pointer         pointer;
//     typedef typename allocator_traits<allocator_type>::const_pointer   const_pointer;
//     typedef typename allocator_traits<allocator_type>::size_type       size_type;
//     typedef typename allocator_traits<allocator_type>::difference_type difference_type;

#include <unordered_map>
#include <type_traits>

#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_map<char, short> C;
        static_assert((std::is_same<C::key_type, char>::value), "");
        static_assert((std::is_same<C::mapped_type, short>::value), "");
        static_assert((std::is_same<C::hasher, std::hash<C::key_type> >::value), "");
        static_assert((std::is_same<C::key_equal, std::equal_to<C::key_type> >::value), "");
        static_assert((std::is_same<C::allocator_type, std::allocator<C::value_type> >::value), "");
        static_assert((std::is_same<C::value_type, std::pair<const C::key_type, C::mapped_type> >::value), "");
        static_assert((std::is_same<C::reference, C::value_type&>::value), "");
        static_assert((std::is_same<C::const_reference, const C::value_type&>::value), "");
        static_assert((std::is_same<C::pointer, C::value_type*>::value), "");
        static_assert((std::is_same<C::const_pointer, const C::value_type*>::value), "");
        static_assert((std::is_same<C::size_type, std::size_t>::value), "");
        static_assert((std::is_same<C::difference_type, std::ptrdiff_t>::value), "");
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<char, short, std::hash<char>, std::equal_to<char>,
                            min_allocator<std::pair<const char, short>>> C;
        static_assert((std::is_same<C::key_type, char>::value), "");
        static_assert((std::is_same<C::mapped_type, short>::value), "");
        static_assert((std::is_same<C::hasher, std::hash<C::key_type> >::value), "");
        static_assert((std::is_same<C::key_equal, std::equal_to<C::key_type> >::value), "");
        static_assert((std::is_same<C::allocator_type, min_allocator<C::value_type> >::value), "");
        static_assert((std::is_same<C::value_type, std::pair<const C::key_type, C::mapped_type> >::value), "");
        static_assert((std::is_same<C::reference, C::value_type&>::value), "");
        static_assert((std::is_same<C::const_reference, const C::value_type&>::value), "");
        static_assert((std::is_same<C::pointer, min_pointer<C::value_type>>::value), "");
        static_assert((std::is_same<C::const_pointer, min_pointer<const C::value_type>>::value), "");
    //  min_allocator doesn't have a size_type, so one gets synthesized
        static_assert((std::is_same<C::size_type, std::make_unsigned<C::difference_type>::type>::value), "");
        static_assert((std::is_same<C::difference_type, std::ptrdiff_t>::value), "");
    }
#endif
}
