//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// insert_iterator

// insert_iterator(Cont& x, Cont::iterator i);

#include <iterator>
#include <vector>
#include "nasty_containers.hpp"

template <class C>
void
test(C c)
{
    std::insert_iterator<C> i(c, c.begin());
}

int main()
{
    test(std::vector<int>());
    test(nasty_vector<int>());
}
