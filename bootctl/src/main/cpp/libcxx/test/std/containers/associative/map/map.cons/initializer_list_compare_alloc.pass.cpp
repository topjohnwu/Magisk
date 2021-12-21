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

// class map

// map(initializer_list<value_type> il, const key_compare& comp, const allocator_type& a);

#include <map>
#include <cassert>
#include "../../../test_compare.h"
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::pair<const int, double> V;
    typedef test_compare<std::less<int> > C;
    typedef test_allocator<std::pair<const int, double> > A;
    std::map<int, double, C, A> m({
                                   {1, 1},
                                   {1, 1.5},
                                   {1, 2},
                                   {2, 1},
                                   {2, 1.5},
                                   {2, 2},
                                   {3, 1},
                                   {3, 1.5},
                                   {3, 2}
                                  }, C(3), A(6));
    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == V(1, 1));
    assert(*next(m.begin()) == V(2, 1));
    assert(*next(m.begin(), 2) == V(3, 1));
    assert(m.key_comp() == C(3));
    assert(m.get_allocator() == A(6));
    }
    {
    typedef std::pair<const int, double> V;
    typedef test_compare<std::less<int> > C;
    typedef min_allocator<std::pair<const int, double> > A;
    std::map<int, double, C, A> m({
                                   {1, 1},
                                   {1, 1.5},
                                   {1, 2},
                                   {2, 1},
                                   {2, 1.5},
                                   {2, 2},
                                   {3, 1},
                                   {3, 1.5},
                                   {3, 2}
                                  }, C(3), A());
    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == V(1, 1));
    assert(*next(m.begin()) == V(2, 1));
    assert(*next(m.begin(), 2) == V(3, 1));
    assert(m.key_comp() == C(3));
    assert(m.get_allocator() == A());
    }
    {
    typedef std::pair<const int, double> V;
    typedef min_allocator<V> A;
    typedef test_compare<std::less<int> > C;
    typedef std::map<int, double, C, A> M;
    A a;
    M m ({ {1, 1},
           {1, 1.5},
           {1, 2},
           {2, 1},
           {2, 1.5},
           {2, 2},
           {3, 1},
           {3, 1.5},
           {3, 2}
          }, a);

    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == V(1, 1));
    assert(*next(m.begin()) == V(2, 1));
    assert(*next(m.begin(), 2) == V(3, 1));
    assert(m.get_allocator() == a);
    }
    {
    typedef std::pair<const int, double> V;
    typedef explicit_allocator<V> A;
    typedef test_compare<std::less<int> > C;
    A a;
    std::map<int, double, C, A> m({
                                   {1, 1},
                                   {1, 1.5},
                                   {1, 2},
                                   {2, 1},
                                   {2, 1.5},
                                   {2, 2},
                                   {3, 1},
                                   {3, 1.5},
                                   {3, 2}
                                  }, C(3), a);
    assert(m.size() == 3);
    assert(distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == V(1, 1));
    assert(*next(m.begin()) == V(2, 1));
    assert(*next(m.begin(), 2) == V(3, 1));
    assert(m.key_comp() == C(3));
    assert(m.get_allocator() == a);
    }
}
