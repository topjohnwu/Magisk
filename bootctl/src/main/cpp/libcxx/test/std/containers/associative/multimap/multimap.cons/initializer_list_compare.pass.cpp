//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <map>

// class multimap

// multimap(initializer_list<value_type> il, const key_compare& comp = key_compare());

#include <map>
#include <cassert>
#include "../../../test_compare.h"
#include "min_allocator.h"

int main()
{
    {
    typedef test_compare<std::less<int> > Cmp;
    typedef std::multimap<int, double, Cmp> C;
    typedef C::value_type V;
    C m(
           {
               {1, 1},
               {1, 1.5},
               {1, 2},
               {2, 1},
               {2, 1.5},
               {2, 2},
               {3, 1},
               {3, 1.5},
               {3, 2}
           },
           Cmp(4)
        );
    assert(m.size() == 9);
    assert(distance(m.begin(), m.end()) == 9);
    C::const_iterator i = m.cbegin();
    assert(*i == V(1, 1));
    assert(*++i == V(1, 1.5));
    assert(*++i == V(1, 2));
    assert(*++i == V(2, 1));
    assert(*++i == V(2, 1.5));
    assert(*++i == V(2, 2));
    assert(*++i == V(3, 1));
    assert(*++i == V(3, 1.5));
    assert(*++i == V(3, 2));
    assert(m.key_comp() == Cmp(4));
    }
    {
    typedef test_compare<std::less<int> > Cmp;
    typedef std::multimap<int, double, Cmp, min_allocator<std::pair<const int, double>>> C;
    typedef C::value_type V;
    C m(
           {
               {1, 1},
               {1, 1.5},
               {1, 2},
               {2, 1},
               {2, 1.5},
               {2, 2},
               {3, 1},
               {3, 1.5},
               {3, 2}
           },
           Cmp(4)
        );
    assert(m.size() == 9);
    assert(distance(m.begin(), m.end()) == 9);
    C::const_iterator i = m.cbegin();
    assert(*i == V(1, 1));
    assert(*++i == V(1, 1.5));
    assert(*++i == V(1, 2));
    assert(*++i == V(2, 1));
    assert(*++i == V(2, 1.5));
    assert(*++i == V(2, 2));
    assert(*++i == V(3, 1));
    assert(*++i == V(3, 1.5));
    assert(*++i == V(3, 2));
    assert(m.key_comp() == Cmp(4));
    }
}
