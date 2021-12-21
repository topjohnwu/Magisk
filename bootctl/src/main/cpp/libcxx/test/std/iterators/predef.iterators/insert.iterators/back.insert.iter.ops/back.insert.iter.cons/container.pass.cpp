//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// back_insert_iterator

// explicit back_insert_iterator(Cont& x);

#include <iterator>
#include <vector>
#include "nasty_containers.hpp"

template <class C>
void
test(C c)
{
    std::back_insert_iterator<C> i(c);
}

int main()
{
    test(std::vector<int>());
    test(nasty_vector<int>());
}
