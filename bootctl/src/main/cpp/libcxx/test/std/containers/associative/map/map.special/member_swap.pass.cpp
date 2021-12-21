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

// void swap(map& m);

#include <map>
#include <cassert>

#include "min_allocator.h"

int main()
{
    typedef std::pair<const int, double> V;
    {
    typedef std::map<int, double> M;
    {
        M m1;
        M m2;
        M m1_save = m1;
        M m2_save = m2;
        m1.swap(m2);
        assert(m1 == m2_save);
        assert(m2 == m1_save);
    }
    {
        V ar2[] =
        {
            V(5, 5),
            V(6, 6),
            V(7, 7),
            V(8, 8),
            V(9, 9),
            V(10, 10),
            V(11, 11),
            V(12, 12)
        };
        M m1;
        M m2(ar2, ar2+sizeof(ar2)/sizeof(ar2[0]));
        M m1_save = m1;
        M m2_save = m2;
        m1.swap(m2);
        assert(m1 == m2_save);
        assert(m2 == m1_save);
    }
    {
        V ar1[] =
        {
            V(1, 1),
            V(2, 2),
            V(3, 3),
            V(4, 4)
        };
        M m1(ar1, ar1+sizeof(ar1)/sizeof(ar1[0]));
        M m2;
        M m1_save = m1;
        M m2_save = m2;
        m1.swap(m2);
        assert(m1 == m2_save);
        assert(m2 == m1_save);
    }
    {
        V ar1[] =
        {
            V(1, 1),
            V(2, 2),
            V(3, 3),
            V(4, 4)
        };
        V ar2[] =
        {
            V(5, 5),
            V(6, 6),
            V(7, 7),
            V(8, 8),
            V(9, 9),
            V(10, 10),
            V(11, 11),
            V(12, 12)
        };
        M m1(ar1, ar1+sizeof(ar1)/sizeof(ar1[0]));
        M m2(ar2, ar2+sizeof(ar2)/sizeof(ar2[0]));
        M m1_save = m1;
        M m2_save = m2;
        m1.swap(m2);
        assert(m1 == m2_save);
        assert(m2 == m1_save);
    }
    }
#if TEST_STD_VER >= 11
    {
    typedef std::map<int, double, std::less<int>, min_allocator<V>> M;
    {
        M m1;
        M m2;
        M m1_save = m1;
        M m2_save = m2;
        m1.swap(m2);
        assert(m1 == m2_save);
        assert(m2 == m1_save);
    }
    {
        V ar2[] =
        {
            V(5, 5),
            V(6, 6),
            V(7, 7),
            V(8, 8),
            V(9, 9),
            V(10, 10),
            V(11, 11),
            V(12, 12)
        };
        M m1;
        M m2(ar2, ar2+sizeof(ar2)/sizeof(ar2[0]));
        M m1_save = m1;
        M m2_save = m2;
        m1.swap(m2);
        assert(m1 == m2_save);
        assert(m2 == m1_save);
    }
    {
        V ar1[] =
        {
            V(1, 1),
            V(2, 2),
            V(3, 3),
            V(4, 4)
        };
        M m1(ar1, ar1+sizeof(ar1)/sizeof(ar1[0]));
        M m2;
        M m1_save = m1;
        M m2_save = m2;
        m1.swap(m2);
        assert(m1 == m2_save);
        assert(m2 == m1_save);
    }
    {
        V ar1[] =
        {
            V(1, 1),
            V(2, 2),
            V(3, 3),
            V(4, 4)
        };
        V ar2[] =
        {
            V(5, 5),
            V(6, 6),
            V(7, 7),
            V(8, 8),
            V(9, 9),
            V(10, 10),
            V(11, 11),
            V(12, 12)
        };
        M m1(ar1, ar1+sizeof(ar1)/sizeof(ar1[0]));
        M m2(ar2, ar2+sizeof(ar2)/sizeof(ar2[0]));
        M m1_save = m1;
        M m2_save = m2;
        m1.swap(m2);
        assert(m1 == m2_save);
        assert(m2 == m1_save);
    }
    }
#endif
}
