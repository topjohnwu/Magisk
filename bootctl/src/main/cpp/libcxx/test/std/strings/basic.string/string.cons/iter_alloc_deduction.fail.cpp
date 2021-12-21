//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5, clang-3.6, clang-3.7, clang-3.8, clang-3.9, clang-4.0
// UNSUPPORTED: apple-clang-6, apple-clang-7, apple-clang-8, apple-clang-9

// template<class InputIterator,
//      class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
//  basic_string(InputIterator, InputIterator, Allocator = Allocator())
//    -> basic_string<typename iterator_traits<InputIterator>::value_type,
//                 char_traits<typename iterator_traits<InputIterator>::value_type>,
//                 Allocator>;
//
//  The deduction guide shall not participate in overload resolution if InputIterator
//  is a type that does not qualify as an input iterator, or if Allocator is a type
//  that does not qualify as an allocator.


#include <string>
#include <iterator>
#include <cassert>
#include <cstddef>

#include "test_macros.h"

class NotAnItertor {};

template <typename T>
struct NotAnAllocator { typedef T value_type; };

int main()
{
    { // Not an iterator at all
    std::basic_string s1{NotAnItertor{}, NotAnItertor{}, std::allocator<char>{}}; // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'basic_string'}}
    }
    { // Not an input iterator
    const char16_t* s = u"12345678901234";
    std::basic_string<char16_t> s0;
    std::basic_string s1{std::back_insert_iterator(s0), //  expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'basic_string'}}
                         std::back_insert_iterator(s0),
                         std::allocator<char16_t>{}};
    }
    { // Not an allocator
    const wchar_t* s = L"12345678901234";
    std::basic_string s1{s, s+10, NotAnAllocator<wchar_t>{}}; // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'basic_string'}}
    }

}
