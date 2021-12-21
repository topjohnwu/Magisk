//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT, class OutputIterator = ostreambuf_iterator<charT> >
// class num_put
//     : public locale::facet
// {
// public:
//     typedef charT          char_type;
//     typedef OutputIterator iter_type;

#include <locale>
#include <iterator>
#include <type_traits>

int main()
{
    static_assert((std::is_base_of<std::locale::facet, std::num_put<char> >::value), "");
    static_assert((std::is_base_of<std::locale::facet, std::num_put<wchar_t> >::value), "");
    static_assert((std::is_same<std::num_put<char>::char_type, char>::value), "");
    static_assert((std::is_same<std::num_put<wchar_t>::char_type, wchar_t>::value), "");
    static_assert((std::is_same<std::num_put<char>::iter_type, std::ostreambuf_iterator<char> >::value), "");
    static_assert((std::is_same<std::num_put<wchar_t>::iter_type, std::ostreambuf_iterator<wchar_t> >::value), "");
}
