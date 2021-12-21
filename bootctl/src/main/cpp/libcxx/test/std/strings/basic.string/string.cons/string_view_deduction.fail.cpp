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
// XFAIL: libcpp-no-deduction-guides

// template<class InputIterator>
//   basic_string(InputIterator begin, InputIterator end,
//   const Allocator& a = Allocator());

// template<class charT,
//          class traits,
//          class Allocator = allocator<charT>
//          >
// basic_string(basic_string_view<charT, traits>, const Allocator& = Allocator())
//   -> basic_string<charT, traits, Allocator>;
//
//  The deduction guide shall not participate in overload resolution if Allocator
//  is a type that does not qualify as an allocator.


#include <string>
#include <string_view>
#include <iterator>
#include <cassert>
#include <cstddef>

int main()
{
    {
    std::string_view sv = "12345678901234";
    std::basic_string s1{sv, 23}; // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'basic_string'}}
    }
}
