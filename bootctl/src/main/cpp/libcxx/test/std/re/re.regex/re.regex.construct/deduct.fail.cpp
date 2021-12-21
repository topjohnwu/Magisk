//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides


// template <class InputIterator, class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
//    vector(InputIterator, InputIterator, Allocator = Allocator())
//    -> vector<typename iterator_traits<InputIterator>::value_type, Allocator>;
//


#include <regex>
#include <string>
#include <iterator>
#include <cassert>
#include <cstddef>


int main()
{
//  Test the explicit deduction guides
    {
//	basic_regex(ForwardIterator, ForwardIterator)
//  <int> is not an iterator
    std::basic_regex re(23, 34);   // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'basic_regex'}}
    }

    {
//	basic_regex(ForwardIterator, ForwardIterator, flag_type)
//  <double> is not an iterator
    std::basic_regex re(23.0, 34.0, std::regex_constants::basic);   // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'basic_regex'}}
    }

//  Test the implicit deduction guides

}
