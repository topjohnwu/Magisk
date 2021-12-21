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

// void swap(multiset& m);

#include <set>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    typedef int V;
    typedef std::multiset<int> M;
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
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12
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
            1,
            2,
            3,
            4
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
            1,
            2,
            3,
            4
        };
        V ar2[] =
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
    typedef int V;
    typedef std::multiset<int, std::less<int>, min_allocator<int>> M;
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
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12
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
            1,
            2,
            3,
            4
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
            1,
            2,
            3,
            4
        };
        V ar2[] =
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
