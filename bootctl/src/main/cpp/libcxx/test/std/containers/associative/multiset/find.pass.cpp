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

//       iterator find(const key_type& k);
// const_iterator find(const key_type& k) const;

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
            typedef M::iterator R;
            V ar[] =
            {
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12
            };
            M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == next(m.begin()));
            r = m.find(7);
            assert(r == next(m.begin(), 2));
            r = m.find(8);
            assert(r == next(m.begin(), 3));
            r = m.find(9);
            assert(r == next(m.begin(), 4));
            r = m.find(10);
            assert(r == next(m.begin(), 5));
            r = m.find(11);
            assert(r == next(m.begin(), 6));
            r = m.find(12);
            assert(r == next(m.begin(), 7));
            r = m.find(4);
            assert(r == next(m.begin(), 8));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12
            };
            const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == next(m.begin()));
            r = m.find(7);
            assert(r == next(m.begin(), 2));
            r = m.find(8);
            assert(r == next(m.begin(), 3));
            r = m.find(9);
            assert(r == next(m.begin(), 4));
            r = m.find(10);
            assert(r == next(m.begin(), 5));
            r = m.find(11);
            assert(r == next(m.begin(), 6));
            r = m.find(12);
            assert(r == next(m.begin(), 7));
            r = m.find(4);
            assert(r == next(m.begin(), 8));
        }
    }
#if TEST_STD_VER >= 11
    {
        typedef int V;
        typedef std::multiset<int, std::less<int>, min_allocator<int>> M;
        {
            typedef M::iterator R;
            V ar[] =
            {
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12
            };
            M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == next(m.begin()));
            r = m.find(7);
            assert(r == next(m.begin(), 2));
            r = m.find(8);
            assert(r == next(m.begin(), 3));
            r = m.find(9);
            assert(r == next(m.begin(), 4));
            r = m.find(10);
            assert(r == next(m.begin(), 5));
            r = m.find(11);
            assert(r == next(m.begin(), 6));
            r = m.find(12);
            assert(r == next(m.begin(), 7));
            r = m.find(4);
            assert(r == next(m.begin(), 8));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                6,
                7,
                8,
                9,
                10,
                11,
                12
            };
            const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
            R r = m.find(5);
            assert(r == m.begin());
            r = m.find(6);
            assert(r == next(m.begin()));
            r = m.find(7);
            assert(r == next(m.begin(), 2));
            r = m.find(8);
            assert(r == next(m.begin(), 3));
            r = m.find(9);
            assert(r == next(m.begin(), 4));
            r = m.find(10);
            assert(r == next(m.begin(), 5));
            r = m.find(11);
            assert(r == next(m.begin(), 6));
            r = m.find(12);
            assert(r == next(m.begin(), 7));
            r = m.find(4);
            assert(r == next(m.begin(), 8));
        }
    }
#endif
#if TEST_STD_VER > 11
    {
    typedef int V;
    typedef std::multiset<V, std::less<>> M;
    typedef M::iterator R;

    V ar[] =
    {
        5,
        6,
        7,
        8,
        9,
        10,
        11,
        12
    };
    M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
    R r = m.find(5);
    assert(r == m.begin());
    r = m.find(6);
    assert(r == next(m.begin()));
    r = m.find(7);
    assert(r == next(m.begin(), 2));
    r = m.find(8);
    assert(r == next(m.begin(), 3));
    r = m.find(9);
    assert(r == next(m.begin(), 4));
    r = m.find(10);
    assert(r == next(m.begin(), 5));
    r = m.find(11);
    assert(r == next(m.begin(), 6));
    r = m.find(12);
    assert(r == next(m.begin(), 7));
    r = m.find(4);
    assert(r == next(m.begin(), 8));
    }

    {
    typedef PrivateConstructor V;
    typedef std::multiset<V, std::less<>> M;
    typedef M::iterator R;

    M m;
    m.insert ( V::make ( 5 ));
    m.insert ( V::make ( 6 ));
    m.insert ( V::make ( 7 ));
    m.insert ( V::make ( 8 ));
    m.insert ( V::make ( 9 ));
    m.insert ( V::make ( 10 ));
    m.insert ( V::make ( 11 ));
    m.insert ( V::make ( 12 ));

    R r = m.find(5);
    assert(r == m.begin());
    r = m.find(6);
    assert(r == next(m.begin()));
    r = m.find(7);
    assert(r == next(m.begin(), 2));
    r = m.find(8);
    assert(r == next(m.begin(), 3));
    r = m.find(9);
    assert(r == next(m.begin(), 4));
    r = m.find(10);
    assert(r == next(m.begin(), 5));
    r = m.find(11);
    assert(r == next(m.begin(), 6));
    r = m.find(12);
    assert(r == next(m.begin(), 7));
    r = m.find(4);
    assert(r == next(m.begin(), 8));
    }
#endif
}
