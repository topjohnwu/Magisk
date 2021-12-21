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

// size_type erase(const key_type& k);

#include <set>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef std::multiset<int> M;
        typedef int V;
        typedef M::size_type I;
        V ar[] =
        {
            3,
            3,
            3,
            5,
            5,
            5,
            7,
            7,
            7
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 9);
        I i = m.erase(6);
        assert(m.size() == 9);
        assert(i == 0);
        assert(*next(m.begin(), 0) == 3);
        assert(*next(m.begin(), 1) == 3);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 5);
        assert(*next(m.begin(), 4) == 5);
        assert(*next(m.begin(), 5) == 5);
        assert(*next(m.begin(), 6) == 7);
        assert(*next(m.begin(), 7) == 7);
        assert(*next(m.begin(), 8) == 7);

        i = m.erase(5);
        assert(m.size() == 6);
        assert(i == 3);
        assert(*next(m.begin(), 0) == 3);
        assert(*next(m.begin(), 1) == 3);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 7);
        assert(*next(m.begin(), 4) == 7);
        assert(*next(m.begin(), 5) == 7);

        i = m.erase(3);
        assert(m.size() == 3);
        assert(i == 3);
        assert(*next(m.begin(), 0) == 7);
        assert(*next(m.begin(), 1) == 7);
        assert(*next(m.begin(), 2) == 7);

        i = m.erase(7);
        assert(m.size() == 0);
        assert(i == 3);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multiset<int, std::less<int>, min_allocator<int>> M;
        typedef int V;
        typedef M::size_type I;
        V ar[] =
        {
            3,
            3,
            3,
            5,
            5,
            5,
            7,
            7,
            7
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 9);
        I i = m.erase(6);
        assert(m.size() == 9);
        assert(i == 0);
        assert(*next(m.begin(), 0) == 3);
        assert(*next(m.begin(), 1) == 3);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 5);
        assert(*next(m.begin(), 4) == 5);
        assert(*next(m.begin(), 5) == 5);
        assert(*next(m.begin(), 6) == 7);
        assert(*next(m.begin(), 7) == 7);
        assert(*next(m.begin(), 8) == 7);

        i = m.erase(5);
        assert(m.size() == 6);
        assert(i == 3);
        assert(*next(m.begin(), 0) == 3);
        assert(*next(m.begin(), 1) == 3);
        assert(*next(m.begin(), 2) == 3);
        assert(*next(m.begin(), 3) == 7);
        assert(*next(m.begin(), 4) == 7);
        assert(*next(m.begin(), 5) == 7);

        i = m.erase(3);
        assert(m.size() == 3);
        assert(i == 3);
        assert(*next(m.begin(), 0) == 7);
        assert(*next(m.begin(), 1) == 7);
        assert(*next(m.begin(), 2) == 7);

        i = m.erase(7);
        assert(m.size() == 0);
        assert(i == 3);
    }
#endif
}
