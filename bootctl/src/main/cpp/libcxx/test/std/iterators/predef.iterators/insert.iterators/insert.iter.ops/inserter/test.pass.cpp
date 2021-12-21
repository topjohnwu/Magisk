//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// template <InsertionContainer Cont>
//   insert_iterator<Cont>
//   inserter(Cont& x, Cont::iterator i);

#include <iterator>
#include <vector>
#include <cassert>
#include "nasty_containers.hpp"

template <class C>
void
test(C c)
{
    std::insert_iterator<C> i = std::inserter(c, c.end());
    i = 0;
    assert(c.size() == 1);
    assert(c.back() == 0);
}

int main()
{
    test(std::vector<int>());
    test(nasty_vector<int>());
}
