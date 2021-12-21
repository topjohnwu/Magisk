//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <experimental/iterator>
//
// template <class _Delim, class _CharT = char, class _Traits = char_traits<_CharT>>
//   class ostream_joiner;
//
//   template<typename T>
//   ostream_joiner & operator=(const T&)
//

#include <experimental/iterator>
#include <iostream>
#include <sstream>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

struct mutating_delimiter {
    mutating_delimiter(char c = ' ') : c_(c) {}
    char get () const { return c_++; }
    mutable char c_;
    };

template<class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>&
operator<<(std::basic_ostream<_CharT, _Traits>& os, const mutating_delimiter &d)
{ return os << d.get(); }


struct mutating_delimiter2 { // same as above, w/o the const and the mutable
    mutating_delimiter2(char c = ' ') : c_(c) {}
    char get () { return c_++; }
    char c_;
    };

template<class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>&
operator<<(std::basic_ostream<_CharT, _Traits>& os, mutating_delimiter2 &d)
{ return os << d.get(); }


namespace exp = std::experimental;

template <class Delim, class Iter, class CharT = char, class Traits = std::char_traits<CharT>>
void test (Delim &&d, Iter first, Iter last, const CharT *expected ) {
    typedef exp::ostream_joiner<typename std::decay<Delim>::type, CharT, Traits> Joiner;

    static_assert((std::is_copy_constructible<Joiner>::value == std::is_copy_constructible<typename std::decay<Delim>::type>::value), "" );
    static_assert((std::is_move_constructible<Joiner>::value == std::is_move_constructible<typename std::decay<Delim>::type>::value), "" );
    static_assert((std::is_copy_assignable<Joiner>   ::value == std::is_copy_assignable<typename std::decay<Delim>::type>   ::value), "" );
    static_assert((std::is_move_assignable<Joiner>   ::value == std::is_move_assignable<typename std::decay<Delim>::type>   ::value), "" );

    std::basic_stringstream<CharT, Traits> sstream;
    Joiner joiner(sstream, d);
    while (first != last)
        *joiner++ = *first++;
    assert(sstream.str() == expected);
    }

int main () {
{
    const char chars[] = "0123456789";
    const int  ints [] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

    test('X', chars, chars+10, "0X1X2X3X4X5X6X7X8X9");
    test('x',  ints,  ints+10, "10x11x12x13x14x15x16x17x18x19");
    test('X', input_iterator<const char*>(chars),         input_iterator<const char*>(chars+10),         "0X1X2X3X4X5X6X7X8X9");
    test('x', input_iterator<const int*>(ints),           input_iterator<const int*>(ints+10),           "10x11x12x13x14x15x16x17x18x19");
    test('X', forward_iterator<const char*>(chars),       forward_iterator<const char*>(chars+10),       "0X1X2X3X4X5X6X7X8X9");
    test('x', forward_iterator<const int*>(ints),         forward_iterator<const int*>(ints+10),         "10x11x12x13x14x15x16x17x18x19");
    test('X', random_access_iterator<const char*>(chars), random_access_iterator<const char*>(chars+10), "0X1X2X3X4X5X6X7X8X9");
    test('x', random_access_iterator<const int*>(ints),   random_access_iterator<const int*>(ints+10),   "10x11x12x13x14x15x16x17x18x19");

    test("Z", chars, chars+10, "0Z1Z2Z3Z4Z5Z6Z7Z8Z9");
    test("z", ints,  ints+10,  "10z11z12z13z14z15z16z17z18z19");

    test<char, const char *, wchar_t> ('X', chars, chars+10, L"0X1X2X3X4X5X6X7X8X9");
    test<char, const int *,  wchar_t> ('x',  ints,  ints+10, L"10x11x12x13x14x15x16x17x18x19");
//  test<char, const char *, char16_t>('X', chars, chars+10, u"0X1X2X3X4X5X6X7X8X9");
//  test<char, const int *,  char16_t>('x',  ints,  ints+10, u"10x11x12x13x14x15x16x17x18x19");
//  test<char, const char *, char32_t>('X', chars, chars+10, U"0X1X2X3X4X5X6X7X8X9");
//  test<char, const int *,  char32_t>('x',  ints,  ints+10, U"10x11x12x13x14x15x16x17x18x19");

    test(mutating_delimiter(),  chars, chars+10, "0 1!2\"3#4$5%6&7'8(9");
    test(mutating_delimiter2(), chars, chars+10, "0 1!2\"3#4$5%6&7'8(9");
    }

    {
    const wchar_t chars[] = L"0123456789";
    const int  ints [] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    test(L'X', chars, chars+10, L"0X1X2X3X4X5X6X7X8X9");
    test(L'x',  ints,  ints+10, L"10x11x12x13x14x15x16x17x18x19");
    test(L'X', input_iterator<const wchar_t*>(chars),         input_iterator<const wchar_t*>(chars+10),         L"0X1X2X3X4X5X6X7X8X9");
    test(L'x', input_iterator<const int*>(ints),              input_iterator<const int*>(ints+10),              L"10x11x12x13x14x15x16x17x18x19");
    test(L'X', forward_iterator<const wchar_t*>(chars),       forward_iterator<const wchar_t*>(chars+10),       L"0X1X2X3X4X5X6X7X8X9");
    test(L'x', forward_iterator<const int*>(ints),            forward_iterator<const int*>(ints+10),            L"10x11x12x13x14x15x16x17x18x19");
    test(L'X', random_access_iterator<const wchar_t*>(chars), random_access_iterator<const wchar_t*>(chars+10), L"0X1X2X3X4X5X6X7X8X9");
    test(L'x', random_access_iterator<const int*>(ints),      random_access_iterator<const int*>(ints+10),      L"10x11x12x13x14x15x16x17x18x19");

    test(L"Z", chars, chars+10, L"0Z1Z2Z3Z4Z5Z6Z7Z8Z9");
    test(L"z", ints,  ints+10,  L"10z11z12z13z14z15z16z17z18z19");

    test<char, const wchar_t *, wchar_t> ('X', chars, chars+10, L"0X1X2X3X4X5X6X7X8X9");
    test<char, const int *,  wchar_t>    ('x',  ints,  ints+10, L"10x11x12x13x14x15x16x17x18x19");

    test(mutating_delimiter(), chars, chars+10, L"0 1!2\"3#4$5%6&7'8(9");
    }

}
