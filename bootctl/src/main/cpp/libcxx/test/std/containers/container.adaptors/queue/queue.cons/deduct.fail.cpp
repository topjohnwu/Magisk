//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides

#include <queue>
#include <list>
#include <iterator>
#include <cassert>
#include <cstddef>


int main()
{
//  Test the explicit deduction guides
    {
//  queue(const Container&, const Alloc&);
//  The '45' is not an allocator
    std::queue que(std::list<int>{1,2,3}, 45);  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'queue'}}
    }

    {
//  queue(const queue&, const Alloc&);
//  The '45' is not an allocator
    std::queue<int> source;
    std::queue que(source, 45);  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'queue'}}
    }

//  Test the implicit deduction guides
    {
//  queue (allocator &)
    std::queue que((std::allocator<int>()));  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'queue'}}
//  Note: The extra parens are necessary, since otherwise clang decides it is a function declaration.
//  Also, we can't use {} instead of parens, because that constructs a
//      stack<allocator<int>, allocator<allocator<int>>>
    }

}
