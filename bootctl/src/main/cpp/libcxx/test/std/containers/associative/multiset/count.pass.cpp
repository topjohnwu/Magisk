//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class multiset

// size_type count(const key_type& k) const;

#include <set>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"
#include "private_constructor.hpp"

int main()
{
    {
    typedef int V;
    typedef std::multiset<int> M;
    {
        typedef M::size_type R;
        V ar[] =
        {
            5,
            5,
            5,
            5,
            7,
            7,
            7,
            9,
            9
        };
        const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.count(4);
        assert(r == 0);
        r = m.count(5);
        assert(r == 4);
        r = m.count(6);
        assert(r == 0);
        r = m.count(7);
        assert(r == 3);
        r = m.count(8);
        assert(r == 0);
        r = m.count(9);
        assert(r == 2);
        r = m.count(10);
        assert(r == 0);
    }
    }
#if TEST_STD_VER >= 11
    {
    typedef int V;
    typedef std::multiset<int, std::less<int>, min_allocator<int>> M;
    {
        typedef M::size_type R;
        V ar[] =
        {
            5,
            5,
            5,
            5,
            7,
            7,
            7,
            9,
            9
        };
        const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.count(4);
        assert(r == 0);
        r = m.count(5);
        assert(r == 4);
        r = m.count(6);
        assert(r == 0);
        r = m.count(7);
        assert(r == 3);
        r = m.count(8);
        assert(r == 0);
        r = m.count(9);
        assert(r == 2);
        r = m.count(10);
        assert(r == 0);
    }
    }
#endif
#if TEST_STD_VER > 11
    {
    typedef int V;
    typedef std::multiset<int, std::less<>> M;
    typedef M::size_type R;
    V ar[] =
    {
        5,
        5,
        5,
        5,
        7,
        7,
        7,
        9,
        9
    };
    const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
    R r = m.count(4);
    assert(r == 0);
    r = m.count(5);
    assert(r == 4);
    r = m.count(6);
    assert(r == 0);
    r = m.count(7);
    assert(r == 3);
    r = m.count(8);
    assert(r == 0);
    r = m.count(9);
    assert(r == 2);
    r = m.count(10);
    assert(r == 0);
    }

    {
    typedef PrivateConstructor V;
    typedef std::multiset<V, std::less<>> M;
    typedef M::size_type R;

    M m;
    m.insert ( V::make ( 5 ));
    m.insert ( V::make ( 5 ));
    m.insert ( V::make ( 5 ));
    m.insert ( V::make ( 5 ));
    m.insert ( V::make ( 7 ));
    m.insert ( V::make ( 7 ));
    m.insert ( V::make ( 7 ));
    m.insert ( V::make ( 9 ));
    m.insert ( V::make ( 9 ));

    R r = m.count(4);
    assert(r == 0);
    r = m.count(5);
    assert(r == 4);
    r = m.count(6);
    assert(r == 0);
    r = m.count(7);
    assert(r == 3);
    r = m.count(8);
    assert(r == 0);
    r = m.count(9);
    assert(r == 2);
    r = m.count(10);
    assert(r == 0);
    }
#endif
}
