//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string_view>

// Test nested types and default template args:

// template<class charT, class traits = char_traits<charT>>
// {
// public:
//     // types:
//     using traits_type               = traits;
//     using value_type                = charT;
//     using pointer                   = value_type*;
//     using const_pointer             = const value_type*;
//     using reference                 = value_type&;
//     using const_reference           = const value_type&;
//     using const_iterator            = implementation-defined ; // see 24.4.2.2
//     using iterator                  = const_iterator;
//     using const_reverse_iterator    = reverse_iterator<const_iterator>;
//     using iterator                  = const_reverse_iterator;
//     using size_type                 = size_t;
//     using difference_type           = ptrdiff_t;
//     static constexpr size_type npos = size_type(-1);
//
// };

#include <string_view>
#include <iterator>
#include <type_traits>

#include "test_macros.h"

template <class Traits>
void
test()
{
    typedef std::basic_string_view<typename Traits::char_type, Traits> S;

    static_assert((std::is_same<typename S::traits_type,     Traits>::value), "");
    static_assert((std::is_same<typename S::value_type,      typename Traits::char_type>::value), "");
    static_assert((std::is_same<typename S::size_type,       std::size_t>::value), "");
    static_assert((std::is_same<typename S::difference_type, ptrdiff_t>::value), "");
    static_assert((std::is_same<typename S::reference,             typename S::value_type&>::value), "");
    static_assert((std::is_same<typename S::const_reference, const typename S::value_type&>::value), "");
    static_assert((std::is_same<typename S::pointer,               typename S::value_type*>::value), "");
    static_assert((std::is_same<typename S::const_pointer,   const typename S::value_type*>::value), "");
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
    static_assert((std::is_same<typename S::iterator,         typename S::const_iterator>::value), "");
    static_assert((std::is_same<typename S::reverse_iterator, typename S::const_reverse_iterator>::value), "");
}

int main()
{
    test<std::char_traits<char> >();
    test<std::char_traits<wchar_t> >();
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    test<std::char_traits<char8_t> >();
#endif
    static_assert((std::is_same<std::basic_string_view<char>::traits_type,
                                std::char_traits<char> >::value), "");
}
