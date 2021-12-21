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
//           class charT = typename iterator_traits< BidirectionalIterator>::value_type,
//           class traits = regex_traits<charT>>
// class regex_token_iterator
// {
// public:
//     typedef basic_regex<charT, traits>       regex_type;
//     typedef sub_match<BidirectionalIterator> value_type;
//     typedef ptrdiff_t                        difference_type;
//     typedef const value_type*                pointer;
//     typedef const value_type&                reference;
//     typedef forward_iterator_tag             iterator_category;

#include <regex>
#include <type_traits>
#include "test_macros.h"

template <class CharT>
void
test()
{
    typedef std::regex_token_iterator<const CharT*> I;
    static_assert((std::is_same<typename I::regex_type, std::basic_regex<CharT> >::value), "");
    static_assert((std::is_same<typename I::value_type, std::sub_match<const CharT*> >::value), "");
    static_assert((std::is_same<typename I::difference_type, std::ptrdiff_t>::value), "");
    static_assert((std::is_same<typename I::pointer, const std::sub_match<const CharT*>*>::value), "");
    static_assert((std::is_same<typename I::reference, const std::sub_match<const CharT*>&>::value), "");
    static_assert((std::is_same<typename I::iterator_category, std::forward_iterator_tag>::value), "");
}

int main()
{
    test<char>();
    test<wchar_t>();
}
