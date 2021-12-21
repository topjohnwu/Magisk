//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides


// template <class InputIterator, class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
//    list(InputIterator, InputIterator, Allocator = Allocator())
//    -> list<typename iterator_traits<InputIterator>::value_type, Allocator>;
//


#include <list>
#include <iterator>
#include <cassert>
#include <cstddef>
#include <climits> // INT_MAX

struct A {};

int main()
{
//  Test the explicit deduction guides

//  Test the implicit deduction guides
    {
//  list (allocator &)
    std::list lst((std::allocator<int>()));  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'list'}}
//  Note: The extra parens are necessary, since otherwise clang decides it is a function declaration.
//  Also, we can't use {} instead of parens, because that constructs a
//      deque<allocator<int>, allocator<allocator<int>>>
    }

}
