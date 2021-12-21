//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// move_iterator

// template <class U>
//   requires HasConstructor<Iter, const U&>
//   move_iterator(const move_iterator<U> &u);

// test requires

#include <iterator>

template <class It, class U>
void
test(U u)
{
    std::move_iterator<U> r2(u);
    std::move_iterator<It> r1 = r2;
}

struct base {};
struct derived {};

int main()
{
    derived d;

    test<base*>(&d);
}
