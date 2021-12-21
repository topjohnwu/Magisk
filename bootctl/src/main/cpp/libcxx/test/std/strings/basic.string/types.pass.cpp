//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// Test nested types and default template args:

// template<class charT, class traits = char_traits<charT>,
//   class Allocator = allocator<charT> >
// {
// public:
//     // types:
//     typedef traits traits_type;
//     typedef typename traits::char_type value_type;
//     typedef Allocator allocator_type;
//     typedef typename Allocator::size_type size_type;
//     typedef typename Allocator::difference_type difference_type;
//     typedef typename Allocator::reference reference;
//     typedef typename Allocator::const_reference const_reference;
//     typedef typename Allocator::pointer pointer;
//     typedef typename Allocator::const_pointer const_pointer;
//     typedef implementation-defined iterator; // See 23.1
//     typedef implementation-defined const_iterator; // See 23.1
//     typedef std::reverse_iterator<iterator> reverse_iterator;
//     typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
//     static const size_type npos = -1;
// };

#include <string>
#include <iterator>
#include <type_traits>

#include "test_traits.h"
#include "test_allocator.h"
#include "min_allocator.h"

template <class Traits, class Allocator>
void
test()
{
    typedef std::basic_string<typename Traits::char_type, Traits, Allocator> S;

    static_assert((std::is_same<typename S::traits_type, Traits>::value), "");
    static_assert((std::is_same<typename S::value_type, typename Traits::char_type>::value), "");
    static_assert((std::is_same<typename S::value_type, typename Allocator::value_type>::value), "");
    static_assert((std::is_same<typename S::allocator_type, Allocator>::value), "");
    static_assert((std::is_same<typename S::size_type, typename std::allocator_traits<Allocator>::size_type>::value), "");
    static_assert((std::is_same<typename S::difference_type, typename std::allocator_traits<Allocator>::difference_type>::value), "");
    static_assert((std::is_same<typename S::reference, typename S::value_type&>::value), "");
    static_assert((std::is_same<typename S::const_reference, const typename S::value_type&>::value), "");
    static_assert((std::is_same<typename S::pointer, typename std::allocator_traits<Allocator>::pointer>::value), "");
    static_assert((std::is_same<typename S::const_pointer, typename std::allocator_traits<Allocator>::const_pointer>::value), "");
    static_assert((std::is_same<
        typename std::iterator_traits<typename S::iterator>::iterator_category,
        std::random_access_iterator_tag>::value), "");
    static_assert((std::is_same<
        typename std::iterator_traits<typename S::const_iterator>::iterator_category,
        std::random_access_iterator_tag>::value), "");
    static_assert((std::is_same<
        typename S::reverse_iterator,
        std::reverse_iterator<typename S::iterator> >::value), "");
    static_assert((std::is_same<
        typename S::const_reverse_iterator,
        std::reverse_iterator<typename S::const_iterator> >::value), "");
    static_assert(S::npos == -1, "");
}

int main()
{
    test<test_traits<char>, test_allocator<char> >();
    test<std::char_traits<wchar_t>, std::allocator<wchar_t> >();
    static_assert((std::is_same<std::basic_string<char>::traits_type,
                                std::char_traits<char> >::value), "");
    static_assert((std::is_same<std::basic_string<char>::allocator_type,
                                std::allocator<char> >::value), "");
#if TEST_STD_VER >= 11
    test<std::char_traits<char>, min_allocator<char> >();
#endif
}
