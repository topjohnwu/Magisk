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

// multimap& operator=(initializer_list<value_type> il);

#include <map>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    typedef std::multimap<int, double> C;
    typedef C::value_type V;
    C m = {{20, 1}};
    m =
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
           };
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
    }
    {
    typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> C;
    typedef C::value_type V;
    C m = {{20, 1}};
    m =
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
           };
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
    }
}
