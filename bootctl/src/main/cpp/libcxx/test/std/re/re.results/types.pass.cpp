//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator,
//           class Allocator = allocator<sub_match<BidirectionalIterator>>>
// class match_results
// {
// public:
//     typedef sub_match<BidirectionalIterator>                  value_type;
//     typedef const value_type&                                 const_reference;
//     typedef const_reference                                   reference;
//     typedef /implementation-defined/                          const_iterator;
//     typedef const_iterator                                    iterator;
//     typedef typename iterator_traits<BidirectionalIterator>::difference_type difference_type;
//     typedef typename allocator_traits<Allocator>::size_type   size_type;
//     typedef Allocator                                         allocator_type;
//     typedef typename iterator_traits<BidirectionalIterator>::value_type char_type;
//     typedef basic_string<char_type>                           string_type;

#include <regex>
#include <type_traits>
#include "test_macros.h"

template <class CharT>
void
test()
{
    typedef std::match_results<CharT*> MR;
    static_assert((std::is_same<typename MR::value_type, std::sub_match<CharT*> >::value), "");
    static_assert((std::is_same<typename MR::const_reference, const std::sub_match<CharT*>& >::value), "");
    static_assert((std::is_same<typename MR::reference, std::sub_match<CharT*>& >::value), "");
    static_assert((!std::is_same<typename MR::const_iterator, void>::value), "");
    static_assert((std::is_same<typename MR::difference_type, std::ptrdiff_t>::value), "");
    static_assert((std::is_same<typename MR::size_type, std::size_t>::value), "");
    static_assert((std::is_same<typename MR::allocator_type, std::allocator<std::sub_match<CharT*> > >::value), "");
    static_assert((std::is_same<typename MR::char_type, CharT>::value), "");
    static_assert((std::is_same<typename MR::string_type, std::basic_string<CharT> >::value), "");
}

int main()
{
    test<char>();
    test<wchar_t>();
}
