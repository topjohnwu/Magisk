//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// Test fails due to use of is_trivially_* trait.
// XFAIL: gcc-4.9

// template <class T, class charT = char, class traits = char_traits<charT>,
//           class Distance = ptrdiff_t>
// class istream_iterator
//     : public iterator<input_iterator_tag, T, Distance, const T*, const T&>
// {
// public:
//     typedef charT char_type;
//     typedef traits traits_type;
//     typedef basic_istream<charT,traits> istream_type;
//     ...
//
// Before C++17, we have:
//   If T is a literal type, then the default constructor shall be a constexpr constructor.
//   If T is a literal type, then this constructor shall be a trivial copy constructor.
//   If T is a literal type, then this destructor shall be a trivial destructor.
// C++17 says:
//   If is_trivially_default_constructible_v<T> is true, then
//       this constructor (the default ctor) is a constexpr constructor.
//   If is_trivially_copy_constructible_v<T> is true, then
//       this constructor (the copy ctor) is a trivial copy constructor.
//   If is_trivially_destructible_v<T> is true, then this
//       destructor is a trivial destructor.
//  Testing the C++17 ctors for this are in the ctor tests.

#include <iterator>
#include <type_traits>
#include <string>

#include "test_macros.h"

int main()
{
    typedef std::istream_iterator<double> I1; // double is trivially destructible
#if TEST_STD_VER <= 14
    static_assert((std::is_convertible<I1,
        std::iterator<std::input_iterator_tag, double, std::ptrdiff_t,
        const double*, const double&> >::value), "");
#else
    static_assert((std::is_same<I1::iterator_category, std::input_iterator_tag>::value), "");
    static_assert((std::is_same<I1::value_type, double>::value), "");
    static_assert((std::is_same<I1::difference_type, std::ptrdiff_t>::value), "");
    static_assert((std::is_same<I1::pointer, const double*>::value), "");
    static_assert((std::is_same<I1::reference, const double&>::value), "");
#endif
    static_assert((std::is_same<I1::char_type, char>::value), "");
    static_assert((std::is_same<I1::traits_type, std::char_traits<char> >::value), "");
    static_assert((std::is_same<I1::istream_type, std::istream>::value), "");
    static_assert( std::is_trivially_copy_constructible<I1>::value, "");
    static_assert( std::is_trivially_destructible<I1>::value, "");

    typedef std::istream_iterator<unsigned, wchar_t> I2; // unsigned is trivially destructible
#if TEST_STD_VER <= 14
    static_assert((std::is_convertible<I2,
        std::iterator<std::input_iterator_tag, unsigned, std::ptrdiff_t,
        const unsigned*, const unsigned&> >::value), "");
#else
    static_assert((std::is_same<I2::iterator_category, std::input_iterator_tag>::value), "");
    static_assert((std::is_same<I2::value_type, unsigned>::value), "");
    static_assert((std::is_same<I2::difference_type, std::ptrdiff_t>::value), "");
    static_assert((std::is_same<I2::pointer, const unsigned*>::value), "");
    static_assert((std::is_same<I2::reference, const unsigned&>::value), "");
#endif
    static_assert((std::is_same<I2::char_type, wchar_t>::value), "");
    static_assert((std::is_same<I2::traits_type, std::char_traits<wchar_t> >::value), "");
    static_assert((std::is_same<I2::istream_type, std::wistream>::value), "");
    static_assert( std::is_trivially_copy_constructible<I2>::value, "");
    static_assert( std::is_trivially_destructible<I2>::value, "");

    typedef std::istream_iterator<std::string> I3; // string is NOT trivially destructible
    static_assert(!std::is_trivially_copy_constructible<I3>::value, "");
    static_assert(!std::is_trivially_destructible<I3>::value, "");
}
