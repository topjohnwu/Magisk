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

// requires CopyConstructible<Cont::value_type>
//   insert_iterator<Cont>&
//   operator=(const Cont::value_type& value);

#include <iterator>
#include <vector>
#include <cassert>
#include "nasty_containers.hpp"

template <class C>
void
test(C c1, typename C::difference_type j,
     typename C::value_type x1, typename C::value_type x2,
     typename C::value_type x3, const C& c2)
{
    std::insert_iterator<C> q(c1, c1.begin() + j);
    q = x1;
    q = x2;
    q = x3;
    assert(c1 == c2);
}

template <class C>
void
insert3at(C& c, typename C::iterator i,
     typename C::value_type x1, typename C::value_type x2,
     typename C::value_type x3)
{
    i = c.insert(i, x1);
    i = c.insert(++i, x2);
    c.insert(++i, x3);
}

int main()
{
    {
    typedef std::vector<int> C;
    C c1;
    for (int i = 0; i < 3; ++i)
        c1.push_back(i);
    C c2 = c1;
    insert3at(c2, c2.begin(), 'a', 'b', 'c');
    test(c1, 0, 'a', 'b', 'c', c2);
    c2 = c1;
    insert3at(c2, c2.begin()+1, 'a', 'b', 'c');
    test(c1, 1, 'a', 'b', 'c', c2);
    c2 = c1;
    insert3at(c2, c2.begin()+2, 'a', 'b', 'c');
    test(c1, 2, 'a', 'b', 'c', c2);
    c2 = c1;
    insert3at(c2, c2.begin()+3, 'a', 'b', 'c');
    test(c1, 3, 'a', 'b', 'c', c2);
    }
    {
    typedef nasty_vector<int> C;
    C c1;
    for (int i = 0; i < 3; ++i)
        c1.push_back(i);
    C c2 = c1;
    insert3at(c2, c2.begin(), 'a', 'b', 'c');
    test(c1, 0, 'a', 'b', 'c', c2);
    c2 = c1;
    insert3at(c2, c2.begin()+1, 'a', 'b', 'c');
    test(c1, 1, 'a', 'b', 'c', c2);
    c2 = c1;
    insert3at(c2, c2.begin()+2, 'a', 'b', 'c');
    test(c1, 2, 'a', 'b', 'c', c2);
    c2 = c1;
    insert3at(c2, c2.begin()+3, 'a', 'b', 'c');
    test(c1, 3, 'a', 'b', 'c', c2);
    }
}
