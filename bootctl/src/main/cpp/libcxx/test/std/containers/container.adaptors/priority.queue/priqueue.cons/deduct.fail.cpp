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
#include <deque>
#include <iterator>
#include <cassert>
#include <cstddef>


int main()
{
//  Test the explicit deduction guides
    {
//  queue(Compare, Container, const Alloc);
//  The '45' is not an allocator
    std::priority_queue pri(std::greater<int>(), std::deque<int>({1,2,3}), 45);  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'priority_queue'}}
    }

    {
//  queue(const queue&, const Alloc&);
//  The '45' is not an allocator
    std::priority_queue<int> source;
    std::priority_queue pri(source, 45);  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'priority_queue'}}
    }

    {
//  priority_queue(Iter, Iter, Comp)
//  int is not an iterator
    std::priority_queue pri(15, 17, std::greater<double>());  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'priority_queue'}}
    }

    {
//  priority_queue(Iter, Iter, Comp, Container)
//  float is not an iterator
    std::priority_queue pri(23.f, 2.f, std::greater<float>(), std::deque<float>());   // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'priority_queue'}}
    }

//  Test the implicit deduction guides
    {
//  priority_queue (allocator &)
    std::priority_queue pri((std::allocator<int>()));  // expected-error {{no viable constructor or deduction guide for deduction of template arguments of 'priority_queue'}}
//  Note: The extra parens are necessary, since otherwise clang decides it is a function declaration.
//  Also, we can't use {} instead of parens, because that constructs a
//      stack<allocator<int>, allocator<allocator<int>>>
    }

}
