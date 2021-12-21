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

// template <class P>
//   iterator insert(P&& p);

#include <map>
#include <cassert>

#include "MoveOnly.h"
#include "min_allocator.h"
#include "test_macros.h"

template <class Container>
void do_insert_rv_test()
{
    typedef std::multimap<int, MoveOnly> M;
    typedef typename M::iterator R;
    typedef typename M::value_type VT;
    M m;
    R r = m.insert(VT(2, 2));
    assert(r == m.begin());
    assert(m.size() == 1);
    assert(r->first == 2);
    assert(r->second == 2);

    r = m.insert(VT(1, 1));
    assert(r == m.begin());
    assert(m.size() == 2);
    assert(r->first == 1);
    assert(r->second == 1);

    r = m.insert(VT(3, 3));
    assert(r == prev(m.end()));
    assert(m.size() == 3);
    assert(r->first == 3);
    assert(r->second == 3);

    r = m.insert(VT(3, 3));
    assert(r == prev(m.end()));
    assert(m.size() == 4);
    assert(r->first == 3);
    assert(r->second == 3);
}

int main()
{
    do_insert_rv_test<std::multimap<int, MoveOnly>>();
    {
        typedef std::multimap<int, MoveOnly, std::less<int>, min_allocator<std::pair<const int, MoveOnly>>> M;
        do_insert_rv_test<M>();
    }
    {
        typedef std::multimap<int, MoveOnly> M;
        typedef M::iterator R;
        M m;
        R r = m.insert({2, MoveOnly(2)});
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(r->first == 2);
        assert(r->second == 2);

        r = m.insert({1, MoveOnly(1)});
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(r->first == 1);
        assert(r->second == 1);

        r = m.insert({3, MoveOnly(3)});
        assert(r == prev(m.end()));
        assert(m.size() == 3);
        assert(r->first == 3);
        assert(r->second == 3);

        r = m.insert({3, MoveOnly(3)});
        assert(r == prev(m.end()));
        assert(m.size() == 4);
        assert(r->first == 3);
        assert(r->second == 3);
    }
}
