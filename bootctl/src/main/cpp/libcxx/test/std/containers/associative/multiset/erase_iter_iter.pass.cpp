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

// iterator erase(const_iterator first, const_iterator last);

#include <set>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef std::multiset<int> M;
        typedef int V;
        typedef M::iterator I;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        I i = m.erase(next(m.cbegin(), 5), next(m.cbegin(), 5));
        assert(m.size() == 8);
        assert(i == next(m.begin(), 5));
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 4);
        assert(*next(m.begin(), 4) == 5);
        assert(*next(m.begin(), 5) == 6);
        assert(*next(m.begin(), 6) == 7);
        assert(*next(m.begin(), 7) == 8);

        i = m.erase(next(m.cbegin(), 3), next(m.cbegin(), 4));
        assert(m.size() == 7);
        assert(i == next(m.begin(), 3));
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 5);
        assert(*next(m.begin(), 4) == 6);
        assert(*next(m.begin(), 5) == 7);
        assert(*next(m.begin(), 6) == 8);

        i = m.erase(next(m.cbegin(), 2), next(m.cbegin(), 5));
        assert(m.size() == 4);
        assert(i == next(m.begin(), 2));
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 7);
        assert(*next(m.begin(), 3) == 8);

        i = m.erase(next(m.cbegin(), 0), next(m.cbegin(), 2));
        assert(m.size() == 2);
        assert(i == next(m.begin(), 0));
        assert(*next(m.begin(), 0) == 7);
        assert(*next(m.begin(), 1) == 8);

        i = m.erase(m.cbegin(), m.cend());
        assert(m.size() == 0);
        assert(i == m.end());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multiset<int, std::less<int>, min_allocator<int>> M;
        typedef int V;
        typedef M::iterator I;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        I i = m.erase(next(m.cbegin(), 5), next(m.cbegin(), 5));
        assert(m.size() == 8);
        assert(i == next(m.begin(), 5));
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 4);
        assert(*next(m.begin(), 4) == 5);
        assert(*next(m.begin(), 5) == 6);
        assert(*next(m.begin(), 6) == 7);
        assert(*next(m.begin(), 7) == 8);

        i = m.erase(next(m.cbegin(), 3), next(m.cbegin(), 4));
        assert(m.size() == 7);
        assert(i == next(m.begin(), 3));
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 5);
        assert(*next(m.begin(), 4) == 6);
        assert(*next(m.begin(), 5) == 7);
        assert(*next(m.begin(), 6) == 8);

        i = m.erase(next(m.cbegin(), 2), next(m.cbegin(), 5));
        assert(m.size() == 4);
        assert(i == next(m.begin(), 2));
        assert(*next(m.begin(), 0) == 1);
        assert(*next(m.begin(), 1) == 2);
        assert(*next(m.begin(), 2) == 7);
        assert(*next(m.begin(), 3) == 8);

        i = m.erase(next(m.cbegin(), 0), next(m.cbegin(), 2));
        assert(m.size() == 2);
        assert(i == next(m.begin(), 0));
        assert(*next(m.begin(), 0) == 7);
        assert(*next(m.begin(), 1) == 8);

        i = m.erase(m.cbegin(), m.cend());
        assert(m.size() == 0);
        assert(i == m.end());
    }
#endif
}
