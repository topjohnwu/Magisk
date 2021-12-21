//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// template <BackInsertionContainer Cont>
//   front_insert_iterator<Cont>
//   front_inserter(Cont& x);

#include <iterator>
#include <list>
#include <cassert>
#include "nasty_containers.hpp"

template <class C>
void
test(C c)
{
    std::front_insert_iterator<C> i = std::front_inserter(c);
    i = 0;
    assert(c.size() == 1);
    assert(c.front() == 0);
}

int main()
{
    test(std::list<int>());
    test(nasty_list<int>());
}
