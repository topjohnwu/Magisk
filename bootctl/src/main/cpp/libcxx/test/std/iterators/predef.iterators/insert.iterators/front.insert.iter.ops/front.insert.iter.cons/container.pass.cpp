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

// explicit front_insert_iterator(Cont& x);

#include <iterator>
#include <list>
#include "nasty_containers.hpp"

template <class C>
void
test(C c)
{
    std::front_insert_iterator<C> i(c);
}

int main()
{
    test(std::list<int>());
    test(nasty_list<int>());
}
