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
//     iterator insert(const_iterator position, P&& p);

#include <map>
#include <cassert>

#include "MoveOnly.h"
#include "min_allocator.h"
#include "test_macros.h"

template <class Container, class Pair>
void do_insert_rv_test()
{
    typedef Container M;
    typedef Pair P;
    typedef typename M::iterator R;
    M m;
    R r = m.insert(m.cend(), P(2, 2));
    assert(r == m.begin());
    assert(m.size() == 1);
    assert(r->first == 2);
    assert(r->second == 2);

    r = m.insert(m.cend(), P(1, 1));
    assert(r == m.begin());
    assert(m.size() == 2);
    assert(r->first == 1);
    assert(r->second == 1);

    r = m.insert(m.cend(), P(3, 3));
    assert(r == prev(m.end()));
    assert(m.size() == 3);
    assert(r->first == 3);
    assert(r->second == 3);

    r = m.insert(m.cend(), P(3, 2));
    assert(r == prev(m.end()));
    assert(m.size() == 4);
    assert(r->first == 3);
    assert(r->second == 2);
}

int main()
{
    do_insert_rv_test<std::multimap<int, MoveOnly>, std::pair<int, MoveOnly> >();
    do_insert_rv_test<std::multimap<int, MoveOnly>, std::pair<const int, MoveOnly> >();

    {
        typedef std::multimap<int, MoveOnly, std::less<int>, min_allocator<std::pair<const int, MoveOnly>>> M;
        typedef std::pair<int, MoveOnly> P;
        typedef std::pair<const int, MoveOnly> CP;
        do_insert_rv_test<M, P>();
        do_insert_rv_test<M, CP>();

    }
    {
        typedef std::multimap<int, MoveOnly> M;
        typedef M::iterator R;
        M m;
        R r = m.insert(m.cend(), {2, MoveOnly(2)});
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(r->first == 2);
        assert(r->second == 2);

        r = m.insert(m.cend(), {1, MoveOnly(1)});
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(r->first == 1);
        assert(r->second == 1);

        r = m.insert(m.cend(), {3, MoveOnly(3)});
        assert(r == prev(m.end()));
        assert(m.size() == 3);
        assert(r->first == 3);
        assert(r->second == 3);

        r = m.insert(m.cend(), {3, MoveOnly(2)});
        assert(r == prev(m.end()));
        assert(m.size() == 4);
        assert(r->first == 3);
        assert(r->second == 2);
    }
}
