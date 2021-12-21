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

//       iterator lower_bound(const key_type& k);
// const_iterator lower_bound(const key_type& k) const;

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
                5,
                5,
                7,
                7,
                7,
                9,
                9,
                9
            };
            M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
            R r = m.lower_bound(4);
            assert(r == next(m.begin(), 0));
            r = m.lower_bound(5);
            assert(r == next(m.begin(), 0));
            r = m.lower_bound(6);
            assert(r == next(m.begin(), 3));
            r = m.lower_bound(7);
            assert(r == next(m.begin(), 3));
            r = m.lower_bound(8);
            assert(r == next(m.begin(), 6));
            r = m.lower_bound(9);
            assert(r == next(m.begin(), 6));
            r = m.lower_bound(11);
            assert(r == next(m.begin(), 9));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                5,
                5,
                7,
                7,
                7,
                9,
                9,
                9
            };
            const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
            R r = m.lower_bound(4);
            assert(r == next(m.begin(), 0));
            r = m.lower_bound(5);
            assert(r == next(m.begin(), 0));
            r = m.lower_bound(6);
            assert(r == next(m.begin(), 3));
            r = m.lower_bound(7);
            assert(r == next(m.begin(), 3));
            r = m.lower_bound(8);
            assert(r == next(m.begin(), 6));
            r = m.lower_bound(9);
            assert(r == next(m.begin(), 6));
            r = m.lower_bound(11);
            assert(r == next(m.begin(), 9));
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
                5,
                5,
                7,
                7,
                7,
                9,
                9,
                9
            };
            M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
            R r = m.lower_bound(4);
            assert(r == next(m.begin(), 0));
            r = m.lower_bound(5);
            assert(r == next(m.begin(), 0));
            r = m.lower_bound(6);
            assert(r == next(m.begin(), 3));
            r = m.lower_bound(7);
            assert(r == next(m.begin(), 3));
            r = m.lower_bound(8);
            assert(r == next(m.begin(), 6));
            r = m.lower_bound(9);
            assert(r == next(m.begin(), 6));
            r = m.lower_bound(11);
            assert(r == next(m.begin(), 9));
        }
        {
            typedef M::const_iterator R;
            V ar[] =
            {
                5,
                5,
                5,
                7,
                7,
                7,
                9,
                9,
                9
            };
            const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
            R r = m.lower_bound(4);
            assert(r == next(m.begin(), 0));
            r = m.lower_bound(5);
            assert(r == next(m.begin(), 0));
            r = m.lower_bound(6);
            assert(r == next(m.begin(), 3));
            r = m.lower_bound(7);
            assert(r == next(m.begin(), 3));
            r = m.lower_bound(8);
            assert(r == next(m.begin(), 6));
            r = m.lower_bound(9);
            assert(r == next(m.begin(), 6));
            r = m.lower_bound(11);
            assert(r == next(m.begin(), 9));
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
        5,
        5,
        7,
        7,
        7,
        9,
        9,
        9
    };
    M m(ar, ar+sizeof(ar)/sizeof(ar[0]));

    R r = m.lower_bound(4);
    assert(r == next(m.begin(), 0));
    r = m.lower_bound(5);
    assert(r == next(m.begin(), 0));
    r = m.lower_bound(6);
    assert(r == next(m.begin(), 3));
    r = m.lower_bound(7);
    assert(r == next(m.begin(), 3));
    r = m.lower_bound(8);
    assert(r == next(m.begin(), 6));
    r = m.lower_bound(9);
    assert(r == next(m.begin(), 6));
    r = m.lower_bound(11);
    assert(r == next(m.begin(), 9));
    }

    {
    typedef PrivateConstructor V;
    typedef std::multiset<V, std::less<>> M;
    typedef M::iterator R;

    M m;
    m.insert ( V::make ( 5 ));
    m.insert ( V::make ( 5 ));
    m.insert ( V::make ( 5 ));
    m.insert ( V::make ( 7 ));
    m.insert ( V::make ( 7 ));
    m.insert ( V::make ( 7 ));
    m.insert ( V::make ( 9 ));
    m.insert ( V::make ( 9 ));
    m.insert ( V::make ( 9 ));

    R r = m.lower_bound(4);
    assert(r == next(m.begin(), 0));
    r = m.lower_bound(5);
    assert(r == next(m.begin(), 0));
    r = m.lower_bound(6);
    assert(r == next(m.begin(), 3));
    r = m.lower_bound(7);
    assert(r == next(m.begin(), 3));
    r = m.lower_bound(8);
    assert(r == next(m.begin(), 6));
    r = m.lower_bound(9);
    assert(r == next(m.begin(), 6));
    r = m.lower_bound(11);
    assert(r == next(m.begin(), 9));
    }
#endif
}
