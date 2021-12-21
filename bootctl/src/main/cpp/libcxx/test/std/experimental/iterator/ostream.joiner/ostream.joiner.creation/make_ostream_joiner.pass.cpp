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
//   template <class _CharT, class _Traits, class _Delim>
//   ostream_joiner<typename decay<_Delim>::type, _CharT, _Traits>
//   make_ostream_joiner(basic_ostream<_CharT, _Traits>& __os, _Delim && __d);
//

#include <experimental/iterator>
#include <iostream>
#include <sstream>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

namespace exp = std::experimental;

template <class Delim, class Iter, class CharT = char, class Traits = std::char_traits<CharT>>
void test (Delim &&d, Iter first, Iter last, const CharT *expected ) {
    std::basic_stringstream<CharT, Traits> sstream;
    auto joiner = exp::make_ostream_joiner(sstream, d);
    typedef exp::ostream_joiner<typename std::decay<Delim>::type, CharT, Traits> Joiner;
    static_assert((std::is_same<decltype(joiner), Joiner>::value), "" );
    while (first != last)
        joiner = *first++;
    assert(sstream.str() == expected);
    }

int main () {
    const char chars[] = "0123456789";
    const int  ints [] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

//  There are more of these tests in another file.
//  This is just to make sure that the ostream_joiner is created correctly
    test('X', chars, chars+10, "0X1X2X3X4X5X6X7X8X9");
    test('x', ints,  ints+10,  "10x11x12x13x14x15x16x17x18x19");
    test("Z", chars, chars+10, "0Z1Z2Z3Z4Z5Z6Z7Z8Z9");
    test("z", ints,  ints+10,  "10z11z12z13z14z15z16z17z18z19");
    }
