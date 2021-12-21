//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <stack>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides


// template <class InputIterator, class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
//    vector(InputIterator, InputIterator, Allocator = Allocator())
//    -> vector<typename iterator_traits<InputIterator>::value_type, Allocator>;
//


#include <stack>
#include <list>
#include <iterator>
#include <cassert>
#include <cstddef>


int main()
{
//  Test the explicit deduction guides
    {
//  stack(const Container&, const Alloc&);
//  The '45' is not an allocator
    std::stack stk(std::list<int>({1,2,3}), 45);  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'stack'}}
    }

    {
//  stack(const stack&, const Alloc&);
//  The '45' is not an allocator
    std::stack<int> source;
    std::stack stk(source, 45);  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'stack'}}
    }

//  Test the implicit deduction guides
    {
//  stack (allocator &)
    std::stack stk((std::allocator<int>()));  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'stack'}}
//  Note: The extra parens are necessary, since otherwise clang decides it is a function declaration.
//  Also, we can't use {} instead of parens, because that constructs a
//      stack<allocator<int>, allocator<allocator<int>>>
    }

}
