//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// front_insert_iterator

// front_insert_iterator<Cont> operator++(int);

#include <iterator>
#include <list>
#include <cassert>
#include "nasty_containers.hpp"

template <class C>
void
test(C c)
{
    std::front_insert_iterator<C> i(c);
    std::front_insert_iterator<C> r = i++;
    r = 0;
    assert(c.size() == 1);
    assert(c.back() == 0);
}

int main()
{
    test(std::list<int>());
    test(nasty_list<int>());
}
