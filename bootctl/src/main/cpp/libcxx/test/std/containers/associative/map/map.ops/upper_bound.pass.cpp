//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

//       iterator upper_bound(const key_type& k);
// const_iterator upper_bound(const key_type& k) const;

#include <map>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"
#include "private_constructor.hpp"

int main()
{
    {
    typedef std::pair<const int, double> V;
    typedef std::map<int, double> M;
    {
        typedef M::iterator R;
        V ar[] =
        {
            V(5, 5),
            V(7, 6),
            V(9, 7),
            V(11, 8),
            V(13, 9),
            V(15, 10),
            V(17, 11),
            V(19, 12)
        };
        M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.upper_bound(5);
        assert(r == next(m.begin(), 1));
        r = m.upper_bound(7);
        assert(r == next(m.begin(), 2));
        r = m.upper_bound(9);
        assert(r == next(m.begin(), 3));
        r = m.upper_bound(11);
        assert(r == next(m.begin(), 4));
        r = m.upper_bound(13);
        assert(r == next(m.begin(), 5));
        r = m.upper_bound(15);
        assert(r == next(m.begin(), 6));
        r = m.upper_bound(17);
        assert(r == next(m.begin(), 7));
        r = m.upper_bound(19);
        assert(r == next(m.begin(), 8));
        r = m.upper_bound(4);
        assert(r == next(m.begin(), 0));
        r = m.upper_bound(6);
        assert(r == next(m.begin(), 1));
        r = m.upper_bound(8);
        assert(r == next(m.begin(), 2));
        r = m.upper_bound(10);
        assert(r == next(m.begin(), 3));
        r = m.upper_bound(12);
        assert(r == next(m.begin(), 4));
        r = m.upper_bound(14);
        assert(r == next(m.begin(), 5));
        r = m.upper_bound(16);
        assert(r == next(m.begin(), 6));
        r = m.upper_bound(18);
        assert(r == next(m.begin(), 7));
        r = m.upper_bound(20);
        assert(r == next(m.begin(), 8));
    }
    {
        typedef M::const_iterator R;
        V ar[] =
        {
            V(5, 5),
            V(7, 6),
            V(9, 7),
            V(11, 8),
            V(13, 9),
            V(15, 10),
            V(17, 11),
            V(19, 12)
        };
        const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.upper_bound(5);
        assert(r == next(m.begin(), 1));
        r = m.upper_bound(7);
        assert(r == next(m.begin(), 2));
        r = m.upper_bound(9);
        assert(r == next(m.begin(), 3));
        r = m.upper_bound(11);
        assert(r == next(m.begin(), 4));
        r = m.upper_bound(13);
        assert(r == next(m.begin(), 5));
        r = m.upper_bound(15);
        assert(r == next(m.begin(), 6));
        r = m.upper_bound(17);
        assert(r == next(m.begin(), 7));
        r = m.upper_bound(19);
        assert(r == next(m.begin(), 8));
        r = m.upper_bound(4);
        assert(r == next(m.begin(), 0));
        r = m.upper_bound(6);
        assert(r == next(m.begin(), 1));
        r = m.upper_bound(8);
        assert(r == next(m.begin(), 2));
        r = m.upper_bound(10);
        assert(r == next(m.begin(), 3));
        r = m.upper_bound(12);
        assert(r == next(m.begin(), 4));
        r = m.upper_bound(14);
        assert(r == next(m.begin(), 5));
        r = m.upper_bound(16);
        assert(r == next(m.begin(), 6));
        r = m.upper_bound(18);
        assert(r == next(m.begin(), 7));
        r = m.upper_bound(20);
        assert(r == next(m.begin(), 8));
    }
    }
#if TEST_STD_VER >= 11
    {
    typedef std::pair<const int, double> V;
    typedef std::map<int, double, std::less<int>, min_allocator<V>> M;
    {
        typedef M::iterator R;
        V ar[] =
        {
            V(5, 5),
            V(7, 6),
            V(9, 7),
            V(11, 8),
            V(13, 9),
            V(15, 10),
            V(17, 11),
            V(19, 12)
        };
        M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.upper_bound(5);
        assert(r == next(m.begin(), 1));
        r = m.upper_bound(7);
        assert(r == next(m.begin(), 2));
        r = m.upper_bound(9);
        assert(r == next(m.begin(), 3));
        r = m.upper_bound(11);
        assert(r == next(m.begin(), 4));
        r = m.upper_bound(13);
        assert(r == next(m.begin(), 5));
        r = m.upper_bound(15);
        assert(r == next(m.begin(), 6));
        r = m.upper_bound(17);
        assert(r == next(m.begin(), 7));
        r = m.upper_bound(19);
        assert(r == next(m.begin(), 8));
        r = m.upper_bound(4);
        assert(r == next(m.begin(), 0));
        r = m.upper_bound(6);
        assert(r == next(m.begin(), 1));
        r = m.upper_bound(8);
        assert(r == next(m.begin(), 2));
        r = m.upper_bound(10);
        assert(r == next(m.begin(), 3));
        r = m.upper_bound(12);
        assert(r == next(m.begin(), 4));
        r = m.upper_bound(14);
        assert(r == next(m.begin(), 5));
        r = m.upper_bound(16);
        assert(r == next(m.begin(), 6));
        r = m.upper_bound(18);
        assert(r == next(m.begin(), 7));
        r = m.upper_bound(20);
        assert(r == next(m.begin(), 8));
    }
    {
        typedef M::const_iterator R;
        V ar[] =
        {
            V(5, 5),
            V(7, 6),
            V(9, 7),
            V(11, 8),
            V(13, 9),
            V(15, 10),
            V(17, 11),
            V(19, 12)
        };
        const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.upper_bound(5);
        assert(r == next(m.begin(), 1));
        r = m.upper_bound(7);
        assert(r == next(m.begin(), 2));
        r = m.upper_bound(9);
        assert(r == next(m.begin(), 3));
        r = m.upper_bound(11);
        assert(r == next(m.begin(), 4));
        r = m.upper_bound(13);
        assert(r == next(m.begin(), 5));
        r = m.upper_bound(15);
        assert(r == next(m.begin(), 6));
        r = m.upper_bound(17);
        assert(r == next(m.begin(), 7));
        r = m.upper_bound(19);
        assert(r == next(m.begin(), 8));
        r = m.upper_bound(4);
        assert(r == next(m.begin(), 0));
        r = m.upper_bound(6);
        assert(r == next(m.begin(), 1));
        r = m.upper_bound(8);
        assert(r == next(m.begin(), 2));
        r = m.upper_bound(10);
        assert(r == next(m.begin(), 3));
        r = m.upper_bound(12);
        assert(r == next(m.begin(), 4));
        r = m.upper_bound(14);
        assert(r == next(m.begin(), 5));
        r = m.upper_bound(16);
        assert(r == next(m.begin(), 6));
        r = m.upper_bound(18);
        assert(r == next(m.begin(), 7));
        r = m.upper_bound(20);
        assert(r == next(m.begin(), 8));
    }
    }
#endif
#if TEST_STD_VER > 11
    {
    typedef std::pair<const int, double> V;
    typedef std::map<int, double, std::less<>> M;
    typedef M::iterator R;
    V ar[] =
    {
        V(5, 5),
        V(7, 6),
        V(9, 7),
        V(11, 8),
        V(13, 9),
        V(15, 10),
        V(17, 11),
        V(19, 12)
    };
    M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
    R r = m.upper_bound(5);
    assert(r == next(m.begin(), 1));
    r = m.upper_bound(7);
    assert(r == next(m.begin(), 2));
    r = m.upper_bound(9);
    assert(r == next(m.begin(), 3));
    r = m.upper_bound(11);
    assert(r == next(m.begin(), 4));
    r = m.upper_bound(13);
    assert(r == next(m.begin(), 5));
    r = m.upper_bound(15);
    assert(r == next(m.begin(), 6));
    r = m.upper_bound(17);
    assert(r == next(m.begin(), 7));
    r = m.upper_bound(19);
    assert(r == next(m.begin(), 8));
    r = m.upper_bound(4);
    assert(r == next(m.begin(), 0));
    r = m.upper_bound(6);
    assert(r == next(m.begin(), 1));
    r = m.upper_bound(8);
    assert(r == next(m.begin(), 2));
    r = m.upper_bound(10);
    assert(r == next(m.begin(), 3));
    r = m.upper_bound(12);
    assert(r == next(m.begin(), 4));
    r = m.upper_bound(14);
    assert(r == next(m.begin(), 5));
    r = m.upper_bound(16);
    assert(r == next(m.begin(), 6));
    r = m.upper_bound(18);
    assert(r == next(m.begin(), 7));
    r = m.upper_bound(20);
    assert(r == next(m.begin(), 8));
    }

    {
    typedef PrivateConstructor PC;
    typedef std::map<PC, double, std::less<>> M;
    typedef M::iterator R;

    M m;
    m [ PC::make(5)  ] = 5;
    m [ PC::make(7)  ] = 6;
    m [ PC::make(9)  ] = 7;
    m [ PC::make(11) ] = 8;
    m [ PC::make(13) ] = 9;
    m [ PC::make(15) ] = 10;
    m [ PC::make(17) ] = 11;
    m [ PC::make(19) ] = 12;

    R r = m.upper_bound(5);
    assert(r == next(m.begin(), 1));
    r = m.upper_bound(7);
    assert(r == next(m.begin(), 2));
    r = m.upper_bound(9);
    assert(r == next(m.begin(), 3));
    r = m.upper_bound(11);
    assert(r == next(m.begin(), 4));
    r = m.upper_bound(13);
    assert(r == next(m.begin(), 5));
    r = m.upper_bound(15);
    assert(r == next(m.begin(), 6));
    r = m.upper_bound(17);
    assert(r == next(m.begin(), 7));
    r = m.upper_bound(19);
    assert(r == next(m.begin(), 8));
    r = m.upper_bound(4);
    assert(r == next(m.begin(), 0));
    r = m.upper_bound(6);
    assert(r == next(m.begin(), 1));
    r = m.upper_bound(8);
    assert(r == next(m.begin(), 2));
    r = m.upper_bound(10);
    assert(r == next(m.begin(), 3));
    r = m.upper_bound(12);
    assert(r == next(m.begin(), 4));
    r = m.upper_bound(14);
    assert(r == next(m.begin(), 5));
    r = m.upper_bound(16);
    assert(r == next(m.begin(), 6));
    r = m.upper_bound(18);
    assert(r == next(m.begin(), 7));
    r = m.upper_bound(20);
    assert(r == next(m.begin(), 8));
    }
#endif
}
