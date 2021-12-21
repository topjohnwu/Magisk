//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// template <class T, class charT = char, class traits = char_traits<charT>,
//           class Distance = ptrdiff_t>
// class ostream_iterator
//     : public iterator<output_iterator_tag, void, void, void, void>
// {
// public:
//     typedef charT char_type;
//     typedef traits traits_type;
//     typedef basic_istream<charT,traits> istream_type;
//     ...

#include <iterator>
#include <type_traits>

#include "test_macros.h"

int main()
{
    typedef std::ostream_iterator<double> I1;
#if TEST_STD_VER <= 14
    static_assert((std::is_convertible<I1,
        std::iterator<std::output_iterator_tag, void, void, void, void> >::value), "");
#else
    static_assert((std::is_same<I1::iterator_category, std::output_iterator_tag>::value), "");
    static_assert((std::is_same<I1::value_type, void>::value), "");
    static_assert((std::is_same<I1::difference_type, void>::value), "");
    static_assert((std::is_same<I1::pointer, void>::value), "");
    static_assert((std::is_same<I1::reference, void>::value), "");
#endif
    static_assert((std::is_same<I1::char_type, char>::value), "");
    static_assert((std::is_same<I1::traits_type, std::char_traits<char> >::value), "");
    static_assert((std::is_same<I1::ostream_type, std::ostream>::value), "");
    typedef std::ostream_iterator<unsigned, wchar_t> I2;
#if TEST_STD_VER <= 14
    static_assert((std::is_convertible<I2,
        std::iterator<std::output_iterator_tag, void, void, void, void> >::value), "");
#else
    static_assert((std::is_same<I2::iterator_category, std::output_iterator_tag>::value), "");
    static_assert((std::is_same<I2::value_type, void>::value), "");
    static_assert((std::is_same<I2::difference_type, void>::value), "");
    static_assert((std::is_same<I2::pointer, void>::value), "");
    static_assert((std::is_same<I2::reference, void>::value), "");
#endif
    static_assert((std::is_same<I2::char_type, wchar_t>::value), "");
    static_assert((std::is_same<I2::traits_type, std::char_traits<wchar_t> >::value), "");
    static_assert((std::is_same<I2::ostream_type, std::wostream>::value), "");
}
