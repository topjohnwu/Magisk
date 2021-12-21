//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator>
// class sub_match
//     : public pair<BidirectionalIterator, BidirectionalIterator>
// {
// public:
//     typedef BidirectionalIterator                               iterator;
//     typedef typename iterator_traits<iterator>::value_type      value_type;
//     typedef typename iterator_traits<iterator>::difference_type difference_type;
//     typedef basic_string<value_type>                            string_type;
//
//     bool matched;
//     ...
// };

#include <regex>
#include <type_traits>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
        typedef std::sub_match<char*> SM;
        static_assert((std::is_same<SM::iterator, char*>::value), "");
        static_assert((std::is_same<SM::value_type, char>::value), "");
        static_assert((std::is_same<SM::difference_type, std::ptrdiff_t>::value), "");
        static_assert((std::is_same<SM::string_type, std::string>::value), "");
        static_assert((std::is_convertible<SM*, std::pair<char*, char*>*>::value), "");

        SM sm;
        sm.first = nullptr;
        sm.second = nullptr;
        sm.matched = false;
    }
    {
        typedef std::sub_match<wchar_t*> SM;
        static_assert((std::is_same<SM::iterator, wchar_t*>::value), "");
        static_assert((std::is_same<SM::value_type, wchar_t>::value), "");
        static_assert((std::is_same<SM::difference_type, std::ptrdiff_t>::value), "");
        static_assert((std::is_same<SM::string_type, std::wstring>::value), "");
        static_assert((std::is_convertible<SM*, std::pair<wchar_t*, wchar_t*>*>::value), "");

        SM sm;
        sm.first = nullptr;
        sm.second = nullptr;
        sm.matched = false;
    }
    {
        static_assert((std::is_same<std::csub_match, std::sub_match<const char*> >::value), "");
        static_assert((std::is_same<std::wcsub_match, std::sub_match<const wchar_t*> >::value), "");
        static_assert((std::is_same<std::ssub_match, std::sub_match<std::string::const_iterator> >::value), "");
        static_assert((std::is_same<std::wssub_match, std::sub_match<std::wstring::const_iterator> >::value), "");
    }
}
