//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// reverse_iterator

// template <class U>
//   requires HasAssign<Iter, const U&>
//   reverse_iterator&
//   operator=(const reverse_iterator<U>& u);

// test requires

#include <iterator>

template <class It, class U>
void
test(U u)
{
    const std::reverse_iterator<U> r2(u);
    std::reverse_iterator<It> r1;
    r1 = r2;
}

struct base {};
struct derived {};

int main()
{
    derived d;
    test<base*>(&d);
}
