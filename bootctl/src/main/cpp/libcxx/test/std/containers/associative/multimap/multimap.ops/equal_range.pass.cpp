//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <multimap>

// class multimap

// pair<iterator, iterator>             equal_range(const key_type& k);
// pair<const_iterator, const_iterator> equal_range(const key_type& k) const;

#include <map>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"
#include "private_constructor.hpp"
#include "is_transparent.h"

int main()
{
    typedef std::pair<const int, double> V;
    {
    typedef std::multimap<int, double> M;
    {
        typedef std::pair<M::iterator, M::iterator> R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.equal_range(4);
        assert(r.first == m.begin());
        assert(r.second == m.begin());
        r = m.equal_range(5);
        assert(r.first == m.begin());
        assert(r.second == next(m.begin(), 3));
        r = m.equal_range(6);
        assert(r.first == next(m.begin(), 3));
        assert(r.second == next(m.begin(), 3));
        r = m.equal_range(7);
        assert(r.first == next(m.begin(), 3));
        assert(r.second == next(m.begin(), 6));
        r = m.equal_range(8);
        assert(r.first == next(m.begin(), 6));
        assert(r.second == next(m.begin(), 6));
        r = m.equal_range(9);
        assert(r.first == next(m.begin(), 6));
        assert(r.second == next(m.begin(), 9));
        r = m.equal_range(10);
        assert(r.first == m.end());
        assert(r.second == m.end());
    }
    {
        typedef std::pair<M::const_iterator, M::const_iterator> R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.equal_range(4);
        assert(r.first == m.begin());
        assert(r.second == m.begin());
        r = m.equal_range(5);
        assert(r.first == m.begin());
        assert(r.second == next(m.begin(), 3));
        r = m.equal_range(6);
        assert(r.first == next(m.begin(), 3));
        assert(r.second == next(m.begin(), 3));
        r = m.equal_range(7);
        assert(r.first == next(m.begin(), 3));
        assert(r.second == next(m.begin(), 6));
        r = m.equal_range(8);
        assert(r.first == next(m.begin(), 6));
        assert(r.second == next(m.begin(), 6));
        r = m.equal_range(9);
        assert(r.first == next(m.begin(), 6));
        assert(r.second == next(m.begin(), 9));
        r = m.equal_range(10);
        assert(r.first == m.end());
        assert(r.second == m.end());
    }
    }
#if TEST_STD_VER >= 11
    {
    typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
    {
        typedef std::pair<M::iterator, M::iterator> R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.equal_range(4);
        assert(r.first == m.begin());
        assert(r.second == m.begin());
        r = m.equal_range(5);
        assert(r.first == m.begin());
        assert(r.second == next(m.begin(), 3));
        r = m.equal_range(6);
        assert(r.first == next(m.begin(), 3));
        assert(r.second == next(m.begin(), 3));
        r = m.equal_range(7);
        assert(r.first == next(m.begin(), 3));
        assert(r.second == next(m.begin(), 6));
        r = m.equal_range(8);
        assert(r.first == next(m.begin(), 6));
        assert(r.second == next(m.begin(), 6));
        r = m.equal_range(9);
        assert(r.first == next(m.begin(), 6));
        assert(r.second == next(m.begin(), 9));
        r = m.equal_range(10);
        assert(r.first == m.end());
        assert(r.second == m.end());
    }
    {
        typedef std::pair<M::const_iterator, M::const_iterator> R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.equal_range(4);
        assert(r.first == m.begin());
        assert(r.second == m.begin());
        r = m.equal_range(5);
        assert(r.first == m.begin());
        assert(r.second == next(m.begin(), 3));
        r = m.equal_range(6);
        assert(r.first == next(m.begin(), 3));
        assert(r.second == next(m.begin(), 3));
        r = m.equal_range(7);
        assert(r.first == next(m.begin(), 3));
        assert(r.second == next(m.begin(), 6));
        r = m.equal_range(8);
        assert(r.first == next(m.begin(), 6));
        assert(r.second == next(m.begin(), 6));
        r = m.equal_range(9);
        assert(r.first == next(m.begin(), 6));
        assert(r.second == next(m.begin(), 9));
        r = m.equal_range(10);
        assert(r.first == m.end());
        assert(r.second == m.end());
    }
    }
#endif
#if TEST_STD_VER > 11
    {
    typedef std::multimap<int, double, std::less<>> M;

    typedef std::pair<M::iterator, M::iterator> R;
    V ar[] =
    {
        V(5, 1),
        V(5, 2),
        V(5, 3),
        V(7, 1),
        V(7, 2),
        V(7, 3),
        V(9, 1),
        V(9, 2),
        V(9, 3)
    };
    M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
    R r = m.equal_range(4);
    assert(r.first == m.begin());
    assert(r.second == m.begin());
    r = m.equal_range(5);
    assert(r.first == m.begin());
    assert(r.second == next(m.begin(), 3));
    r = m.equal_range(6);
    assert(r.first == next(m.begin(), 3));
    assert(r.second == next(m.begin(), 3));
    r = m.equal_range(7);
    assert(r.first == next(m.begin(), 3));
    assert(r.second == next(m.begin(), 6));
    r = m.equal_range(8);
    assert(r.first == next(m.begin(), 6));
    assert(r.second == next(m.begin(), 6));
    r = m.equal_range(9);
    assert(r.first == next(m.begin(), 6));
    assert(r.second == next(m.begin(), 9));
    r = m.equal_range(10);
    assert(r.first == m.end());
    assert(r.second == m.end());

    r = m.equal_range(C2Int(4));
    assert(r.first == m.begin());
    assert(r.second == m.begin());
    r = m.equal_range(C2Int(5));
    assert(r.first == m.begin());
    assert(r.second == next(m.begin(), 3));
    r = m.equal_range(C2Int(6));
    assert(r.first == next(m.begin(), 3));
    assert(r.second == next(m.begin(), 3));
    r = m.equal_range(C2Int(7));
    assert(r.first == next(m.begin(), 3));
    assert(r.second == next(m.begin(), 6));
    r = m.equal_range(C2Int(8));
    assert(r.first == next(m.begin(), 6));
    assert(r.second == next(m.begin(), 6));
    r = m.equal_range(C2Int(9));
    assert(r.first == next(m.begin(), 6));
    assert(r.second == next(m.begin(), 9));
    r = m.equal_range(C2Int(10));
    assert(r.first == m.end());
    assert(r.second == m.end());
    }

    {
    typedef PrivateConstructor PC;
    typedef std::multimap<PC, double, std::less<>> M;
    typedef std::pair<M::iterator, M::iterator> R;

    M m;
    m.insert ( std::make_pair<PC, double> ( PC::make(5), 1 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(5), 2 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(5), 3 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(7), 1 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(7), 2 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(7), 3 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(9), 1 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(9), 2 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(9), 3 ));

//  assert(m.size() == 9);
    R r = m.equal_range(4);
    assert(r.first == m.begin());
    assert(r.second == m.begin());
    r = m.equal_range(5);
    assert(r.first == m.begin());
    assert(r.second == next(m.begin(), 3));
    r = m.equal_range(6);
    assert(r.first == next(m.begin(), 3));
    assert(r.second == next(m.begin(), 3));
    r = m.equal_range(7);
    assert(r.first == next(m.begin(), 3));
    assert(r.second == next(m.begin(), 6));
    r = m.equal_range(8);
    assert(r.first == next(m.begin(), 6));
    assert(r.second == next(m.begin(), 6));
    r = m.equal_range(9);
    assert(r.first == next(m.begin(), 6));
    assert(r.second == next(m.begin(), 9));
    r = m.equal_range(10);
    assert(r.first == m.end());
    assert(r.second == m.end());
    }
#endif
}
