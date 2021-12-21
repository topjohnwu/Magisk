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

// back_insert_iterator<Cont> operator++(int);

#include <iterator>
#include <vector>
#include <cassert>
#include "nasty_containers.hpp"

template <class C>
void
test(C c)
{
    std::back_insert_iterator<C> i(c);
    std::back_insert_iterator<C> r = i++;
    r = 0;
    assert(c.size() == 1);
    assert(c.back() == 0);
}

int main()
{
    test(std::vector<int>());
    test(nasty_vector<int>());
}
